//
// Created by Burg on 11.05.2023.
//
#include "OBJ_parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

using namespace std;

const regex OBJ_parser::vertexRegex(R"(v (-?\d*.?\d*) (-?\d.?\d*) (-?\d.?\d*))");
const std::regex OBJ_parser::normalRegex(R"(vn (-?\d*.?\d*) (-?\d.?\d*) (-?\d.?\d*))");
const std::regex OBJ_parser::textureRegex(R"(vt (-?\d*.?\d*) (-?\d.?\d*))");
const std::regex OBJ_parser::triangleRegex(R"(f (\d*)/(\d*)/(\d*) (\d*)/(\d*)/(\d*) (\d*)/(\d*)/(\d*))");


void OBJ_parser::loadFromFile(string path) {
    ifstream file (path);
    if (!file.is_open()) {
        throw runtime_error("error: could not open mesh at" + path);
    }

    string line;
    while (getline(file,line)) {
        smatch matches;
        if (std::regex_match(line, matches, triangleRegex)) { // triangle
            triangle_vertices.emplace_back(stoi(matches.str(1)) - 1, stoi(matches.str(4))- 1, stoi(matches.str(7))- 1);
            triangle_textureCords.emplace_back(stoi(matches.str(2))- 1, stoi(matches.str(5))- 1, stoi(matches.str(8))- 1);
            triangle_normals.emplace_back(stoi(matches.str(3))- 1, stoi(matches.str(6))- 1, stoi(matches.str(9))- 1);
        } else if (std::regex_match(line, matches, vertexRegex)) { // vertex
            vertices.emplace_back(stod(matches.str(1)), stod(matches.str(2)), stod(matches.str(3)));
        } else if (std::regex_match(line, matches, normalRegex)) { // normals
            normals.emplace_back(stod(matches.str(1)), stod(matches.str(2)), stod(matches.str(3)));
        } else if (std::regex_match(line, matches, textureRegex)) { // texture
            textureCords.emplace_back(stod(matches.str(1)), stod(matches.str(2)));
        }
    }
}