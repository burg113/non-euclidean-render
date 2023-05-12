#include <iostream>
#include "lib/toml.hpp"
#include "src/ConfigParser.h"
#include "src/OBJ_parser.h"
#include "src/GeoGraph.h"
#include "lib/stb_image_write.h"
#include <filesystem>
#include <set>

typedef unsigned char u8;

const char pathSeperator = '/';

using namespace std;



int main(int argc, char **argv) {
    string fileDir;
    if (argc > 1)
        fileDir = argv[1];

    ConfigParser configParser;
    configParser.loadConfig(fileDir);

    OBJ_parser objParser;
    objParser.loadFromFile(configParser.meshPath);


    cout << objParser.triangle_vertices.size() << "\n";
    GeoGraph graph(objParser.vertices, objParser.triangle_vertices);
    int w = 500, h = 500, comp = 3;
    vector<u8> data(w * h * comp);
    State state;
    state.tri = 0;
    state.pos = graph.triangles[0].getMid();
    const float scale = 0.05;
    set<int> triangleSet;
    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            state.dir.x = (j - w / 2.0f) * scale;
            state.dir.y = (h / 2.0f - i) * scale;
            float dist = state.dir.len();
            state.dir = state.dir.normalized();
            State res = graph.traverse(state, dist);
            int t = res.tri;
            triangleSet.insert(t);
            data[3*(w * i + j)]   = (42 * t) % 255;
            data[3*(w * i + j) + 1] = (244 * (t==0)) % 255;
            data[3*(w * i + j) + 2] = (143 * t + 42) % 255;
        }
    }
//    for(int i : triangleSet) cout << i << "\n";

    stbi_write_png("rendered.png", w, h, comp, data.data(), 0);
    /*for (auto a : objParser.vertices) {
        cout << get<0>(a) << " " << get<1>(a) << " " << get<2>(a) << "\n";
    }*/

    return 0;

}