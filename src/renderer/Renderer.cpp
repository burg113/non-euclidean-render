//
// Created by Burg on 13.05.2023.
//
#include <fstream>
#include <time.h>
#include <thread>
#include <queue>
#include "Renderer.h"
#include "../../lib/stb_image_write.h"


Renderer::Renderer(int w, int h, GeoGraph graph, RenderingTarget &target) : w(w), h(h), threadContext(w, h, graph),
                                                                            renderingTarget(target) {
    top = vector<Ray>(w);
    bottom = vector<Ray>(w);
    left = vector<Ray>(h - 2);
    right = vector<Ray>(h - 2);

    for (int i = 0; i < w; i++) {
        top[i].direction = Vec2d((i + 0.5) - w / 2.0, -h / 2.0).normalized();
        bottom[i].direction = Vec2d(w / 2.0 - (i + 0.5), h / 2.0).normalized();
    }
    for (int i = 0; i < h - 2; i++) {
        right[i].direction = Vec2d(w / 2.0, (h - 1) / 2.0 - (i + 0.5)).normalized();
        left[i].direction = Vec2d(-w / 2.0, (i + 0.5) - (h - 1) / 2.0).normalized();
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
                        Vec2d(x + 0.5, y + 0.5).len(), iy * w + ix);
            }
                // upper triangle (y+0.5)/abs(x+0.5) < - h / w
            else if ((y + 0.5) * w <= -h * abs(x + 0.5)) {
                bottom[clamp(w / 2 + (x * h + h / 2) / (y * 2 + 1), 0, w - 1)].renderPoints.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(), iy * w + ix);
            }
                // right triangle (x+0.5)/(abs(y+0.5)) > w/h
            else if ((x + 0.5) * h >= w * abs(y + 0.5)) {
                // y * w /x

                right[clamp(h / 2 + (y * w + w / 2) / (x * 2 + 1), 0, h - 3)].renderPoints.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(),
                        iy * w + ix);
            }
                // left triangle (x+0.5)/(abs(y+0.5)) < -w/h
            else {
                left[clamp(h / 2 + (y * w + w / 2) / (x * 2 + 1), 0, h - 3)].renderPoints.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(),
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
        cout << "rt: awake" << endl;
        while (true) {

            // cout << "rt: fetching data" << endl;
            threadContext.renderQueueMutex.lock();
            // cout << "rt: locked render queue" << endl;

            while (threadContext.renderQueue.empty()) {
                threadContext.renderQueueMutex.unlock();
                // cout << "rt: unlocked render queue" << endl;
                //cout << "rt: trying to lock newDataCondMutex" << endl;
                unique_lock<mutex> ul(threadContext.newDataCondMutex,defer_lock_t());
                ul.lock();
                cout << "rt: waiting" << endl;
                threadContext.newDataCond.wait(ul,[&](){
                    threadContext.renderQueueMutex.lock();
                    bool b = threadContext.renderQueue.empty();
                    threadContext.renderQueueMutex.unlock();
                    return !b;
                });
                ul.unlock();
                // cout << "rt: received new data" << endl;
                threadContext.renderQueueMutex.lock();
            }

            RenderChunk chunk = threadContext.renderQueue.front();
            threadContext.renderQueue.pop();
            threadContext.renderQueueMutex.unlock();
            // cout << "rt: unlocked" << endl;
            // cout << "rt: got chunk " << chunk.scale << endl;

            int frame = chunk.frame;
            State startState = chunk.start;
            vector<Ray *> &rays = *chunk.rays;
            auto t1 = chrono::high_resolution_clock::now();

            // cout << "rt: starting batch" << endl;
            int countAdd = 0;
            for (auto ray: rays) {
                State state = startState;
                state.dir = ray->direction;
                float lastDist = 0;
                float nextHit = -1;

                countAdd += 3 * (int)ray->renderPoints.size();
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

                    // writing out data todo: possible problems with poping by main render thread at same time???
                    // no mutex needed as no two threads should render the same part of the image
                    chunk.rb->pixel[3*index  ] = u8(127 + 120 * threadContext.graph.triangles[state.tri].normal3d.x);
                    chunk.rb->pixel[3*index+1] = u8(127 + 120 * threadContext.graph.triangles[state.tri].normal3d.y);
                    chunk.rb->pixel[3*index+2] = u8(127 + 120 * threadContext.graph.triangles[state.tri].normal3d.z);
                    lastDist = dist;
                }
            }
            chunk.rb->notifyCount(countAdd);
            // cout << "rt: finished batch" << endl;
            auto t2 = chrono::high_resolution_clock::now();
            // cout << "t: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << "ms" << endl;
        }

    };

    renderThreads = vector<thread>(10);
    for (int i = 0; i < renderThreadAmount; i++)
        renderThreads.emplace_back(renderThread, ref(threadContext));

    auto outThread = [](RenderingTarget &target, ThreadContext &threadContext) {

        while (true) {
            auto t1 = chrono::high_resolution_clock::now();

            //cout << "# waiting for a new frame" << endl;
            while (threadContext.renderBuffers.empty())
                int a = 0;
            //cout << "# waiting for frame to finish rendering..." << endl;
            threadContext.renderBuffers.front()->waitFull();
            //cout << "# got full frame" << endl;
            RenderBuffer *rb = threadContext.renderBuffers.front();
            vector<u8> data = rb->pixel;
            // comment out for disabling writing to file:
            target.writeOut(pair<int, int>(threadContext.width, threadContext.height), data);

            //cout << "# rendered full frame" << endl;
            delete rb;
            threadContext.renderBuffers.pop_front();
            auto t2 = chrono::high_resolution_clock::now();
            cout << "# frame took: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << "ms" << endl;
        }
    };
    mainThread = thread(outThread, ref(renderingTarget), ref(threadContext));
}

void Renderer::renderDebug(State start, double scale, const vector<LoggingTarget *> &loggingTargets,
                           map<string, string> info) {

    vector<string> debugInfo;
    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    auto log = [&](string str) {
        for (LoggingTarget *target: loggingTargets)
            (*target).writeOutNewLine(str);
    };

    log("This is a log from " + string(ctime(&currentTime)));
    log("resolution        : " + to_string(w) + " x " + to_string(h));
    log("");
    log("mesh              : " + info["mesh"]);
    log("triangles (mesh)  : " + info["triangles"]);
    log("vertices (mesh)   : " + info["vertices"]);
    log("triangles (graph) : " + to_string(threadContext.graph.triangles.size()));
    log("");
    log("scale             : " + to_string(int(scale)) + "." + to_string(int(scale * 10) % 10));
    for (const string &str: debugInfo)
        cout << str << endl;

    auto t1 = chrono::high_resolution_clock::now();
    ConsoleOut out = ConsoleOut();
    render(start, scale, {&out});
    auto t2 = chrono::high_resolution_clock::now();

    long long renderTimeMs = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    log("rendering took    : " + to_string(renderTimeMs) + "ms");
    log("pixel per second  : " + to_string((long long) w * h * 1000 / renderTimeMs));
}

void Renderer::render(State startState, double scale, const vector<LoggingTarget *> &loggingTargets) {
    auto log = [&](string str) {
        for (LoggingTarget *target: loggingTargets)
            (*target).writeOut(str);
    };

    vector<u8> data(w * h * 3);

    cout << "m initializing render: " << endl;
    cout << "m adding new renderBuffer..." << endl;
    RenderBuffer* renderBuffer = new RenderBuffer(frameCount, threadContext.width * threadContext.height * 3);
    threadContext.renderBuffers.push_back(renderBuffer);
    cout << "m finished" << endl;

    cout << "locking mutex..." << endl;
    threadContext.renderQueueMutex.lock();
    cout << "m finished" << endl;
    for (vector<Ray *> &rays: chunks) {
        RenderChunk chunk;
        chunk.frame = frameCount;
        chunk.rb = renderBuffer;
        chunk.start = startState;
        chunk.scale = scale;
        chunk.rays = &rays;
        threadContext.renderQueue.push(chunk);
    }
    cout << "m added chunks" << endl;
    threadContext.renderQueueMutex.unlock();
    cout << "m unlocked mutex" << endl;
    frameCount++;
    threadContext.newDataCond.notify_all();
    cout << "m notified all" << endl;
}

