//
// Created by Burg on 13.05.2023.
//
#include <fstream>
#include <time.h>
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


void Renderer::debugRender(State start, double scale, const vector<LoggingTarget *> &loggingTargets, map<string, string> info) {
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
    log("scale             : " + to_string(int(scale))+"." + to_string(int(scale*10)%10));
    for (const string &str: debugInfo)
        cout << str << endl;

    auto t1 = chrono::high_resolution_clock::now();
    ConsoleOut out = ConsoleOut();
    render(start, scale, {&out});
    auto t2 = chrono::high_resolution_clock::now();

    long long renderTimeMs = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    log("rendering took    : " + to_string(renderTimeMs) + "ms");
    log("pixel per second  : " + to_string(w * h* 1000 / renderTimeMs ));
}

void Renderer::render(State start, double scale, const vector<LoggingTarget *> &loggingTargets) {
    auto log = [&](string str) {
        for (LoggingTarget *target: loggingTargets)
            (*target).writeOut(str);
    };
    State state = start;
    vector<u8> data(w * h * 3);
    int count = 0;
    for (int i = 0; i < h; i++) {
        while (count * h < (100 * i)) {
            count++;
            if (count % 10 == 0)
                log("#");
            else
                log(".");
        }

        for (int j = 0; j < w; j++) {
            state.dir.x = (j - w / 2.0) / scale / w * 100; // *0.005f
            state.dir.y = (h / 2.0 - i) / scale / w * 100;
            float dist = state.dir.len();
            state.dir = state.dir.normalized();

            State res = graph.traverse(state, dist);

            Vec3d normal3d = graph.triangles[res.tri].normal3d;
            data[3 * (w * i + j)] = u8(127 + 120 * normal3d.x);
            data[3 * (w * i + j) + 1] = u8(127 + 120 * normal3d.y);
            data[3 * (w * i + j) + 2] = u8(127 + 120 * normal3d.z);
        }
    }
    log("\n");

    renderingTarget.writeOut({w, h}, data);
}

