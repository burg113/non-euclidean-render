//
// Created by Burg on 13.05.2023.
//
#include <fstream>
#include <time.h>
#include <thread>
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

Renderer::Renderer(int w, int h, GeoGraph graph, RenderingTarget &target) : w(w), h(h), graph(std::move(graph)),
                                                                            renderingTarget(target) {
    top = vector<pair<Vec2d, vector<pair<float, int>>>>(w);
    bottom = vector<pair<Vec2d, vector<pair<float, int>>>>(w);
    left = vector<pair<Vec2d, vector<pair<float, int>>>>(h - 2);
    right = vector<pair<Vec2d, vector<pair<float, int>>>>(h - 2);

    for (int i = 0; i < w; i++) {
        top[i].first = Vec2d((i + 0.5) - w / 2.0, -h / 2.0).normalized();
        top[i].second.reserve(h / 4 * 3);
        bottom[i].first = Vec2d(w / 2.0 - (i + 0.5), h / 2.0).normalized();
        bottom[i].second.reserve(h / 4 * 3);
    }
    for (int i = 0; i < h - 2; i++) {
        right[i].first = Vec2d(w / 2.0, (h - 1) / 2.0 - (i + 0.5)).normalized();
        bottom[i].second.reserve(w / 4 * 3);
        left[i].first = Vec2d(-w / 2.0, (i + 0.5) - (h - 1) / 2.0).normalized();
        bottom[i].second.reserve(w / 4 * 3);
    }

    // assuming w, h even
    for (int y = -h / 2; y < (h + 1) / 2; y++) {
        for (int x = -w / 2; x < (w + 1) / 2; x++) {
            int ix = x + w / 2;
            int iy = y + h / 2;

            // lower triangle:(y+0.5)/(abs(x+0.5)) > h / w
            if ((y + 0.5) * w >= h * abs(x + 0.5)) {
                // x * h/y = i
                top[clamp(w / 2 + (x * h + h / 2) / (y * 2 + 1), 0, w - 1)].second.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(), iy * w + ix);
            }
                // upper triangle (y+0.5)/abs(x+0.5) < - h / w
            else if ((y + 0.5) * w <= -h * abs(x + 0.5)) {
                bottom[clamp(w / 2 + (x * h + h / 2) / (y * 2 + 1), 0, w - 1)].second.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(), iy * w + ix);
            }
                // right triangle (x+0.5)/(abs(y+0.5)) > w/h
            else if ((x + 0.5) * h >= w * abs(y + 0.5)) {
                // y * w /x

                right[clamp(h / 2 + (y * w + w / 2) / (x * 2 + 1), 0, h - 3)].second.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(),
                        iy * w + ix);
            }
                // left triangle (x+0.5)/(abs(y+0.5)) < -w/h
            else {
                left[clamp(h / 2 + (y * w + w / 2) / (x * 2 + 1), 0, h - 3)].second.emplace_back(
                        Vec2d(x + 0.5, y + 0.5).len(),
                        iy * w + ix);
            }
        }

    }

    auto sortSubVector = [](auto &vec) {
        for (auto &a: vec)
            std::sort(a.second.begin(), a.second.end());
    };
    sortSubVector(top);
    sortSubVector(bottom);
    sortSubVector(right);
    sortSubVector(left);
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
    int count = 0;
    auto renderPart = [&](vector<pair<Vec2d, vector<pair<float, int>>>> &vec /*,GeoGraph graph*/) {
        count = 0;
        for (auto &borderPixel: vec) {
            State state = startState;
            state.dir = borderPixel.first;
            float lastDist = 0;
            float nextHit = -1;

            double trueScaleInverse = 100 / (scale * w);
            for (auto &[dist, index]: borderPixel.second) {
                float rayDist = (dist - lastDist) * trueScaleInverse;
                if (rayDist < nextHit) {
                    nextHit -= rayDist;
                    state.pos += state.dir * rayDist;
                } else {
                    tie(state, nextHit) = graph.traverse(state, rayDist);
                }
                data[3 * index] = u8(127 + 120 * graph.triangles[state.tri].normal3d.x);
                data[3 * index + 1] = u8(127 + 120 * graph.triangles[state.tri].normal3d.y);
                data[3 * index + 2] = u8(127 + 120 * graph.triangles[state.tri].normal3d.z);
                lastDist = dist;
            }
            count++;
        }
    };


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


    log("\n");

    renderingTarget.writeOut({w, h}, data);
}

