#include <iostream>
#include "lib/toml.hpp"
#include "src/io/ConfigParser.h"
#include "src/io/OBJ_parser.h"
#include "src/renderer/GeoGraph.h"
#include "lib/stb_image_write.h"
#include "src/renderer/Renderer.h"

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

    cout << "parsing obj... ";
    OBJ_parser objParser;
    objParser.loadFromFile(configParser.meshPath);
    cout << "finished " << endl;

    GeoGraph graph(objParser.vertices, objParser.triangleVertices, objParser.uv, objParser.triangleUV);

    auto time = chrono::high_resolution_clock::now().time_since_epoch();
    string outPath = configParser.outPath + getFileName(configParser.meshPath) + "/";
    outPath+= to_string((int)configParser.scale) + "/";
    outPath+= to_string(time.count() / 100000000) + "/";

    FileOut renderingTarget(outPath, configParser.outFileName + ".png");
    Renderer renderer(configParser.width, configParser.height, graph, renderingTarget);

    State state;
    state.tri = 0;
    state.pos = graph.triangles[state.tri].getMid();

    FileOut logFile(outPath, configParser.outFileName + ".log");
    logFile.threadSave = true;
    logFile.debugLevel =  DebugLevel::IMPORTANT; // disabeling most debugs (otherwise there will be ~1.500 lines per frame)

    ConsoleOut consoleOut = ConsoleOut();
    consoleOut.threadSave = true;
    consoleOut.debugToStderr = true;
    consoleOut.disableDebug = false;
    consoleOut.debugLevel = DebugLevel::IMPORTANT; // disabeling most debugs (otherwise there will be ~1.500 lines per frame)


    vector<LoggingTarget *> loggingTargets = vector<LoggingTarget *>();

    // comment out for disabling writing to file:
    renderer.addLoggingTarget(&logFile);
    renderer.addLoggingTarget(&consoleOut);

    for (int i=0;i<10;i++)
        renderer.render(state, configParser.scale);
    renderer.mainThread.join();

    cout << "\n" << "done!";
    return 0;

}