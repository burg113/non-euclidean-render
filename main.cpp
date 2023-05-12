#include <iostream>
#include "lib/toml.hpp"
#include "src/ConfigParser.h"
#include "src/OBJ_parser.h"
#include "src/GeoGraph.h"
#include "lib/stb_image_write.h"
#include <filesystem>
#include <set>
#include <chrono>

typedef unsigned char u8;

const char pathSeperator = '/';

using namespace std;


int main(int argc, char **argv) {
    string fileDir;
    if (argc > 1)
        fileDir = argv[1];

    ConfigParser configParser;
    configParser.loadConfig(fileDir);

    cout << "starting to parse obj... " << endl;
    OBJ_parser objParser;
    objParser.loadFromFile(configParser.meshPath);
    cout << "finished " << endl;


    cout << "object has : " << objParser.triangle_vertices.size() << " triangles\n";
    GeoGraph graph(objParser.vertices, objParser.triangle_vertices);
    int w = 1920, h = 1080, comp = 3;
    vector<u8> data(w * h * comp);
    State state;
    state.tri = 0;
    state.pos = graph.triangles[state.tri].getMid();

    cout << "starting render: ";
    auto t1 = chrono::high_resolution_clock::now();
    int count = 0;
//    cout << graph.triangles[state.tri] << "\n";
    const float scale = 200;
    set<int> triangleSet;
    for (int i = 0; i < h; i++) {
        while (count * h < (100 * i)) {
            count++;
            if (count % 10 == 0)
                cout << "#";
            else
                cout << ".";
        }
        for (int j = 0; j < w; j++) {
            state.dir.x = (j - w / 2.0f) / scale / w * 100; // *0.005f
            state.dir.y = (h / 2.0f - i) / scale / w * 100;
            float dist = state.dir.len();
            state.dir = state.dir.normalized();
            State res = graph.traverse(state, dist);
            Vec3d normal3d = graph.triangles[res.tri].normal3d;
//            triangleSet.insert(res.tri);
            data[3 * (w * i + j)] = u8(127 + 120 * normal3d.x);
            data[3 * (w * i + j) + 1] = u8(127 + 120 * normal3d.y);
            data[3 * (w * i + j) + 2] = u8(127 + 120 * normal3d.z);
        }
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "\n" << "finished in " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() <<"ms\n";
//    for(int i : triangleSet) cout << i << "\n";

    cout << "\n" << "writing to file: " << configParser.outPath + configParser.outFileName << endl;
    stbi_write_png((configParser.outPath + configParser.outFileName).c_str(), w, h, comp, data.data(), 0);

    cout << "\n" << "done!";
    return 0;

}