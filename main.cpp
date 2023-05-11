#include <iostream>
#include "lib/toml.hpp"
#include "src/ConfigParser.h"
#include "src/OBJ_parser.h"
#include <filesystem>

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

    /*for (auto a : objParser.vertices) {
        cout << get<0>(a) << " " << get<1>(a) << " " << get<2>(a) << "\n";
    }*/

    return 0;

}