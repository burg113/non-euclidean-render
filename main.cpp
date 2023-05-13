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


    cout << "resolution      : " << configParser.width << " x " << configParser.height << "\n";
    cout << "mesh            : " << configParser.meshPath << "\n";
    cout << "triangle count  : " << objParser.triangle_vertices.size() << "\n";
    cout << "vertex count    : " << objParser.vertices.size() << "\n";
    cout << "scale           : " << configParser.scale << "\n";
    GeoGraph graph(objParser.vertices, objParser.triangle_vertices);
    int width = configParser.width, height = configParser.height, comp = 3;
    vector<u8> data(configParser.width * configParser.height * comp);
    State state;
    state.tri = 0;
    state.pos = graph.triangles[state.tri].getMid();

    cout << "rendering       : ";
    auto t1 = chrono::high_resolution_clock::now();
    int count = 0;
//    cout << graph.triangles[state.tri] << "\n";
    const double scale = configParser.scale;
    set<int> triangleSet;
    for (int i = 0; i < height; i++) {
        while (count * height < (100 * i)) {
            count++;
            if (count % 10 == 0)
                cout << "#";
            else
                cout << ".";
        }
        for (int j = 0; j < width; j++) {
            state.dir.x = (j - width / 2.0) / scale / width * 100; // *0.005f
            state.dir.y = (height / 2.0 - i) / scale / width * 100;
            float dist = state.dir.len();
            state.dir = state.dir.normalized();
            State res = graph.traverse(state, dist);
            Vec3d normal3d = graph.triangles[res.tri].normal3d;
//            triangleSet.insert(res.tri);
            data[3 * (width * i + j)] = u8(127 + 120 * normal3d.x);
            data[3 * (width * i + j) + 1] = u8(127 + 120 * normal3d.y);
            data[3 * (width * i + j) + 2] = u8(127 + 120 * normal3d.z);
        }
    }
    cout << endl;
    auto t2 = chrono::high_resolution_clock::now();
    long long renderTime = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
    cout << "time taken      : " << renderTime << "ms (" << renderTime / 1000 << "." << renderTime % 1000 << "s)\n";

    // writing image file
    auto b = chrono::high_resolution_clock::now().time_since_epoch();
    int startInd = configParser.meshPath.find_last_of(configParser.pathSeparator) + 1;
    string dir = configParser.outPath +
                 configParser.meshPath.substr(startInd, configParser.meshPath.find_last_of(".") - startInd)
                 + "/" + to_string((int)scale) + "/" + to_string(b.count() / 100000000);
    filesystem::create_directories(dir);
    string outFilePath = dir + "/" + configParser.outFileName;
    cout << "\n" << "writing to file: " << outFilePath << endl;
    stbi_write_png((outFilePath + ".png").c_str(), width, height, comp, data.data(), 0);

    // writing log file
    ofstream myfile;
    myfile.open(outFilePath + ".log");
    auto a = chrono::system_clock::to_time_t(chrono::high_resolution_clock::now());
    myfile << "This is a log from " << ctime(&a);
    myfile << "resolution      : " << width << " x " << height << "\n\n";
    myfile << "mesh            : " << configParser.meshPath << "\n";
    myfile << "triangle count  : " << objParser.triangle_vertices.size() << "\n";
    myfile << "vertex count    : " << objParser.vertices.size() << "\n\n";
    myfile << "scale           : " << scale << "\n\n";
    myfile << "time taken      : " << renderTime << "ms (" << renderTime / 1000 << "." << renderTime % 1000 << "s)\n";
    myfile.close();

    cout << "\n" << "done!";
    return 0;

}