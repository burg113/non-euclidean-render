//
// Created by Burg on 13.05.2023.
//
#include <fstream>
#include <thread>
#include <queue>
#include <utility>
#include "Renderer.h"

using namespace std;
using glm::vec2, glm::normalize, glm::length;

void ThreadContext::log(std::string str) {
    for (LoggingTarget *target: loggingTargets)
        target->writeOutNewLine(str);
};

void ThreadContext::debug(std::string str) {
    for (LoggingTarget *target: loggingTargets)
        target->debugNewLine(str);
};

void ThreadContext::debug(std::string str, int level) {
    for (LoggingTarget *target: loggingTargets)
        target->debugNewLine(str, level);
};

ThreadContext::ThreadContext(int width, int height, GeoGraph graph) : width(width), height(height),
                                                                      graph(std::move(graph)) {};

RenderChunk::RenderChunk(int frame, RenderBuffer *rb, const State &start, double scale, std::vector<Ray *> *rays)
        : frame(frame), rb(rb), start(start), scale(scale), rays(rays) {};


RenderBuffer::RenderBuffer(int frame, int size) : frame(frame) {
    pixel = std::vector<u8>(size, 48);
}

void RenderBuffer::notifyCount(int add) {
    countMut.lock();
    count += add;
    if (count == pixel.size()) {
        fullCond.notify_all();
        full = true;
    }
    countMut.unlock();
}

void RenderBuffer::waitFull() {
    std::unique_lock<std::mutex> lock(fullMut, std::defer_lock_t());
    lock.lock();
    if (!full)
        fullCond.wait(lock);
    lock.unlock();
}


Renderer::Renderer(int w, int h, GeoGraph graph, RenderingTarget &target) :
        w(w), h(h), threadContext(w, h, graph), renderingTarget(target) {
    top = vector<Ray>(w);
    bottom = vector<Ray>(w);
    left = vector<Ray>(h - 2);
    right = vector<Ray>(h - 2);

    for (int i = 0; i < w; i++) {
        top[i].direction = normalize(vec2((i + 0.5) - w / 2.0, -h / 2.0));
        bottom[i].direction = normalize(vec2(w / 2.0 - (i + 0.5), h / 2.0));
    }
    for (int i = 0; i < h - 2; i++) {
        right[i].direction = normalize(vec2(w / 2.0, (h - 1) / 2.0 - (i + 0.5)));
        left[i].direction = normalize(vec2(-w / 2.0, (i + 0.5) - (h - 1) / 2.0));
    }

    // assuming w, h even
    for (int y = -h / 2; y < (h + 1) / 2; y++) {
        for (int x = -w / 2; x < (w + 1) / 2; x++) {
            int ix = x + w / 2;
            int iy = y + h / 2;

            // lower triangle:(y+0.5)/(abs(x+0.5)) > h / w
            if ((y + 0.5) * w >= h * abs(x + 0.5)) {
                // x * h/y = i
                top[clamp(w / 2 + (x * h + h / 2) / (y * 2 + 1), 0, w - 1)].renderPoints.emplace_back(
                        length(vec2(x + 0.5, y + 0.5)), iy * w + ix);
            }
                // upper triangle (y+0.5)/abs(x+0.5) < - h / w
            else if ((y + 0.5) * w <= -h * abs(x + 0.5)) {
                bottom[clamp(w / 2 + (x * h + h / 2) / (y * 2 + 1), 0, w - 1)].renderPoints.emplace_back(
                        length(vec2(x + 0.5, y + 0.5)), iy * w + ix);
            }
                // right triangle (x+0.5)/(abs(y+0.5)) > w/h
            else if ((x + 0.5) * h >= w * abs(y + 0.5)) {
                // y * w /x

                right[clamp(h / 2 + (y * w + w / 2) / (x * 2 + 1), 0, h - 3)].renderPoints.emplace_back(
                        length(vec2(x + 0.5, y + 0.5)),
                        iy * w + ix);
            }
                // left triangle (x+0.5)/(abs(y+0.5)) < -w/h
            else {
                left[clamp(h / 2 + (y * w + w / 2) / (x * 2 + 1), 0, h - 3)].renderPoints.emplace_back(
                        length(vec2(x + 0.5, y + 0.5)),
                        iy * w + ix);
            }
        }

    }

    auto sortSubVector = [](vector<Ray> &vec) {
        for (auto &a: vec) {
            std::sort(a.renderPoints.begin(), a.renderPoints.end());
            a.renderPoints.shrink_to_fit();
        }
    };
    sortSubVector(top);
    sortSubVector(bottom);
    sortSubVector(right);
    sortSubVector(left);

    auto splitChunks = [&](vector<Ray> &list) {
        int count = 0;
        vector<Ray *> current;
        for (Ray &a: list) {
            count += a.renderPoints.size();
            current.push_back(&a);
            if (count > pixelPerChunk) {
                count = 0;
                chunks.push_back(current);
                current = vector<Ray *>();
            }
        }

        if (!current.empty())
            chunks.push_back(current);
    };
    splitChunks(top);
    splitChunks(bottom);
    splitChunks(right);
    splitChunks(left);

    auto renderThread = [](ThreadContext &threadContext) {
        threadContext.debug("rt: awake");
        while (true) {

            threadContext.debug("rt: fetching data", 100);
            threadContext.renderQueueMutex.lock();
            threadContext.debug("rt: locked render queue", 100);

            while (threadContext.renderQueue.empty()) {
                threadContext.renderQueueMutex.unlock();
                threadContext.debug("rt: unlocked render queue", 100);
                threadContext.debug("rt: trying to lock newDataCondMutex", 100);
                unique_lock<mutex> ul(threadContext.newDataCondMutex, defer_lock_t());
                ul.lock();
                threadContext.debug("rt: waiting", 1);
                threadContext.newDataCond.wait(ul, [&]() {
                    threadContext.renderQueueMutex.lock();
                    bool b = threadContext.renderQueue.empty();
                    threadContext.renderQueueMutex.unlock();
                    return !b;
                });
                ul.unlock();
                threadContext.debug("rt: received new data", 1);
                threadContext.renderQueueMutex.lock();
            }

            RenderChunk chunk = threadContext.renderQueue.front();
            threadContext.renderQueue.pop();
            threadContext.renderQueueMutex.unlock();
            threadContext.debug("rt: unlocked", 100);
            threadContext.debug("rt: got chunk " + to_string(int(chunk.scale))
                                + "." + to_string(int(chunk.scale * 10) % 10), 100);
            threadContext.debug("rt: rendering frame:  " + to_string(chunk.frame), 100);

            State startState = chunk.start;
            vector<Ray *> &rays = *chunk.rays;
            auto t1 = chrono::high_resolution_clock::now();

            threadContext.debug("rt: starting batch", 100);
            int countAdd = 0;
            for (auto ray: rays) {
                State state = startState;
                state.dir = ray->direction;
                float lastDist = 0;
                float nextHit = -1;

                countAdd += 3 * (int) ray->renderPoints.size();
                double trueScaleInverse = 100 / (chunk.scale * threadContext.width);
                for (auto &[dist, index]: ray->renderPoints) {
                    // traversing graph until the next point is hit
                    float rayDist = (dist - lastDist) * trueScaleInverse;
                    if (rayDist < nextHit) {
                        nextHit -= rayDist;
                        state.pos += state.dir * rayDist;
                    } else {
                        tie(state, nextHit) = threadContext.graph.traverse(state, rayDist);
                    }

                    // writing out data - no mutex needed as no two threads should render the same part of the image
                    chunk.rb->pixel[3 * index] = u8(127 + 120 * threadContext.graph.triangles[state.tri].normal3d.x);
                    chunk.rb->pixel[3 * index + 1] = u8(
                            127 + 120 * threadContext.graph.triangles[state.tri].normal3d.y);
                    chunk.rb->pixel[3 * index + 2] = u8(
                            127 + 120 * threadContext.graph.triangles[state.tri].normal3d.z);
                    lastDist = dist;
                }
            }
            chunk.rb->notifyCount(countAdd);
            threadContext.debug("rt: finished batch", 100);
            auto t2 = chrono::high_resolution_clock::now();
            threadContext.debug(
                    "rt: took:" + to_string(chrono::duration_cast<chrono::milliseconds>(t2 - t1).count()) + "ms", 100);
        }

    };

    renderThreads = vector<thread>(100);
    for (int i = 0; i < renderThreadAmount; i++)
        renderThreads.emplace_back(renderThread, ref(threadContext));

    auto outThread = [](RenderingTarget &target, ThreadContext &threadContext) {

        while (true) {
            auto t1 = chrono::high_resolution_clock::now();
            threadContext.debug("# waiting for a new frame", 1);
            while (threadContext.renderBuffers.empty())
                int a = 0;
            threadContext.debug("# waiting for frame to finish rendering...", 1);
            threadContext.renderBuffers.front()->waitFull();
            threadContext.debug("# got full frame", 1);
            RenderBuffer *rb = threadContext.renderBuffers.front();
            vector<u8> data = rb->pixel;
            // comment out for disabling writing to file:
            target.writeOut(pair<int, int>(threadContext.width, threadContext.height), data);

            threadContext.debug("# rendered full frame", 1);

            delete rb;
            threadContext.renderBuffers.pop_front();
            auto t2 = chrono::high_resolution_clock::now();
            threadContext.log("# frame took: " + to_string(
                    chrono::duration_cast<chrono::microseconds>(t2 - t1).count() / 1000) + "ms");
        }
    };
    mainThread = thread(outThread, ref(renderingTarget), ref(threadContext));
}

void Renderer::render(State startState, double scale) {
    vector<u8> data(w * h * 3);

    threadContext.debug("m initializing render");
    threadContext.debug("m adding new renderBuffer...", 1);
    RenderBuffer *renderBuffer = new RenderBuffer(frameCount, threadContext.width * threadContext.height * 3);
    threadContext.renderBuffers.push_back(renderBuffer);

    threadContext.debug("m locking mutex...", 5);
    threadContext.renderQueueMutex.lock();
    threadContext.debug("m finished", 5);
    for (vector<Ray *> &rays: chunks) {
        RenderChunk chunk(frameCount, renderBuffer, startState, scale, &rays);
        threadContext.renderQueue.push(chunk);
    }
    threadContext.debug("m added chunks", 5);
    threadContext.renderQueueMutex.unlock();
    threadContext.debug("m unlocked mutex", 5);
    frameCount++;
    threadContext.newDataCond.notify_all();
    threadContext.debug("m notified all", 5);
}

void Renderer::addLoggingTarget(LoggingTarget *target) {
    threadContext.loggingTargets.push_back(target);
}



