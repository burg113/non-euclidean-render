#include <iostream>
#include "src/io/ConfigParser.h"
#include "src/io/OBJ_parser.h"
#include "src/renderer/GeoGraph.h"
#include "src/racing_game/RacingGame.h"


#include <SDL.h>

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

    /*
    auto time = chrono::high_resolution_clock::now().time_since_epoch();
    string outPath = configParser.outPath + getFileName(configParser.meshPath) + "/";
    outPath+= to_string((int)configParser.scale) + "/";
    outPath+= to_string(time.count() / 100000000) + "/";
     */

    // FileOut fileRenderTarget(outPath, configParser.outFileName + ".png");
    ScreenOut screen(configParser.width, configParser.height, "Non euclidean renderer");
    Renderer renderer(configParser.width, configParser.height, graph, screen);


    /*FileOut logFile(outPath, configParser.outFileName + ".log");
    logFile.threadSave = true;
    logFile.debugLevel =  DebugLevel::IMPORTANT; // disabeling most debugs (otherwise there will be ~1.500 lines per frame)


    ConsoleOut consoleOut = ConsoleOut();
    consoleOut.threadSave = false; // way faster
    consoleOut.debugToStderr = true;
    consoleOut.disableDebug = true;
    consoleOut.debugLevel = DebugLevel::IMPORTANT; // disabeling most debugs (otherwise there will be ~1.500 lines per frame)


    // comment out for disabling writing to file:
    // renderer.addLoggingTarget(&logFile);
    renderer.addLoggingTarget(&consoleOut);
    */

    Texture texture(configParser.texturePath);

    RacingGame racingGame(renderer, graph, texture);
    racingGame.run();

    cout << "\n" << "done!";
    return 0;

}