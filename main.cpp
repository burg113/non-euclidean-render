#include <iostream>
#include "lib/toml.hpp"
#include "src/ConfigParser.h"
#include "src/OBJ_parser.h"
#include "src/GeoGraph.h"
#include "lib/stb_image_write.h"
#include "src/Renderer.h"

typedef unsigned char u8;

const char pathSeparator = '/';

using namespace std;

string getFileName(string path) {
    int start = path.find_last_of(pathSeparator) + 1;
    return path.substr(start, path.find_last_of('.') -start);
}

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

    GeoGraph graph(objParser.vertices, objParser.triangle_vertices);


    auto time = chrono::high_resolution_clock::now().time_since_epoch();
    string outPath = configParser.outPath + getFileName(configParser.meshPath) + "/";
    outPath+= to_string((int)configParser.scale) + "/";
    outPath+= to_string(time.count() / 100000000) + "/";

    FileOut renderingTarget(outPath, configParser.outFileName + ".png");
    Renderer renderer(configParser.width, configParser.height, graph, renderingTarget);

    State state;
    state.tri = 0;
    state.pos = graph.triangles[state.tri].getMid();

    map<string, string> debugInfo;

    debugInfo["mesh"] = getFileName(configParser.meshPath);
    debugInfo["triangles"] = to_string(objParser.triangle_vertices.size());
    debugInfo["vertices"] = to_string(objParser.vertices.size());

    FileOut logFile(outPath, configParser.outFileName + ".log");
    ConsoleOut consoleOut = ConsoleOut();
    vector<LoggingTarget *> loggingTargets = vector<LoggingTarget *>();
    loggingTargets.push_back(&logFile);
    loggingTargets.push_back(&consoleOut);

    renderer.renderDebug(state, configParser.scale, loggingTargets, debugInfo);
    //renderer.render(state,configParser.scale);

    cout << "\n" << "done!";
    return 0;

}