//
// Created by Burg on 13.05.2023.
//
#include <fstream>
#include <time.h>
#include <thread>
#include <queue>
#include "Renderer.h"
#include "../lib/stb_image_write.h"

void FileOut::writeOut(pair<int, int> resolution, vector<u8> &data) {
    filesystem::create_directories(path);
    stbi_write_png((path + name).c_str(), resolution.first, resolution.second, 3, data.data(), 0);
}

void FileOut::writeOut(std::string &str) {
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    file << str;
    file.close();
}


void FileOut::writeOutNewLine(std::string &str) {
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    file << str << "\n";
    file.close();
}

void FileOut::writeOut(vector<std::string> &data) {
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    for (string str: data) {
        file << str << "\n";
    }
    file.close();
}


void ConsoleOut::writeOut(vector<string> &data) {
    for (auto str: data)
        cout << str << "\n";
}

void ConsoleOut::writeOut(string &str) {
    cout << str;
}

void ConsoleOut::writeOutNewLine(std::string &str) {
    cout << str << "\n";
}

Renderer::Renderer(int w, int h, GeoGraph graph, RenderingTarget &target) : w(w), h(h), threadContext(w,h,graph),
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

    auto splitChunks= [&](vector<Ray> &list) {
        int count = 0;
        vector<Ray*> current;
        for (auto a : list) {
            count += a.renderPoints.size();
            current.push_back(&a);
            if (count > pixelPerChunk) {
                count =0;
                chunks.push_back(current);
                current = vector<Ray*>();
            }
        }
        if (!current.empty())
            chunks.push_back(current);
    };
    splitChunks(top);
    splitChunks(bottom);
    splitChunks(right);
    splitChunks(left);

    auto renderThread = [] (ThreadContext &threadContext) {};
        while (true) {
            threadContext.dataAccesMutex.lock();

            if (threadContext.renderQueue.empty()) {
                threadContext.dataAccesMutex.unlock();
                unique_lock<mutex> ul;
                threadContext.newDataCond.wait(ul);
                threadContext.dataAccesMutex.lock();
            }

            RenderChunk chunk = threadContext.renderQueue.front();
            threadContext.renderQueue.pop();
            threadContext.dataAccesMutex.unlock();

            State startState = chunk.start;
            vector<Ray*> rays = chunk.rays;
            for (auto ray: rays) {
                State state = startState;
                state.dir = ray->direction;
                float lastDist = 0;
                float nextHit = -1;

                double trueScaleInverse = 100 / (chunk.scale * threadContext.width);
                for (auto &[dist, index]: ray->renderPoints) {
                    float rayDist = (dist - lastDist) * trueScaleInverse;
                    if (rayDist < nextHit) {
                        nextHit -= rayDist;
                        state.pos += state.dir * rayDist;
                    } else {
                        tie(state, nextHit) = graph.traverse(state, rayDist);
                    }
                    // figure out which renderBuffer to write to

                    /*threadContext.renderBuffer[0].setRange(3 * index, {u8(127 + 120 * graph.triangles[state.tri].normal3d.x),
                                                                       u8(127 + 120 * graph.triangles[state.tri].normal3d.y)},
                                                                        u8(127 + 120 * graph.triangles[state.tri].normal3d.z))
                    */
                    //data[3 * index] = u8(127 + 120 * graph.triangles[state.tri].normal3d.x);
                    //data[3 * index + 1] = u8(127 + 120 * graph.triangles[state.tri].normal3d.y);
                    //data[3 * index + 2] = u8(127 + 120 * graph.triangles[state.tri].normal3d.z);
                    lastDist = dist;
                }
            }


        }

    };
    //renderThreads = vector<thread>(10,thread(renderThread,ref(threadContext)));


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
    log("triangles (graph) : " + to_string(graph.triangles.size()));
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


    thread t1 = thread(renderPart, ref(top));
    thread t2 = thread(renderPart, ref(bottom));
    thread t3 = thread(renderPart, ref(right));
    thread t4 = thread(renderPart, ref(left));
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    /*auto a1 = chrono::system_clock::now();
    renderPart(top);
    renderPart(bottom);
    renderPart(right);
    renderPart(left);
    auto a2 = chrono::system_clock::now();
    cout << (a2 - a1).count();*/

    // renderingTarget.writeOut({w, h}, data);
}


                                                                dataAccesMutex(dataAccesMutex), chunks(chunks),
                                                                newDataCond(newDataCond) {}

