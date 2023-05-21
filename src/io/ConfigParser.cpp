//
// Created by Burg on 11.05.2023.
//
#include <iostream>
#include "toml.hpp"
#include "ConfigParser.h"

using namespace std;
template <typename T>
// read string from given toml table
T readFromTable(toml::table &table, vector<string> path, T stdReturn, string errorMsg = "") {
    toml::node_view a = table[path[0]];
    for (int i = 0; i < path.size() - 1; i++)
        a = a[path[i + 1]];
    auto s = a.value<T>();
    if (!s.has_value()) {
        if (errorMsg.size()>0)
            cout << errorMsg << "\n";
        return stdReturn;
    }else
        return s.value();
}



void ConfigParser::loadConfig(string tryFirst) {
    while (true) {
        string input = tryFirst;
        if (!tryFirst.empty()) {
            input = tryFirst;
            tryFirst = "";
        } else {
            cout << "please input the path to the scene file: \n";
            getline(cin, input);
        }

        optional<toml::table> t = readToml(input);
        if (!t.has_value())
            continue;
        toml::table table = t.value();

        string basePath = input.substr(0, input.find_last_of(pathSeparator) + 1);

        meshPath = basePath + readFromTable(table, {"mesh", "file"}, "","error: missing mesh path");
        texturePath = basePath + readFromTable(table, {"mesh", "texture"}, "", "error: missing texture path");
        outPath = basePath + readFromTable(table, {"output", "path"},"");
        outFileName = readFromTable(table, {"output", "filename"},"");
        scale = readFromTable(table, {"render", "scale"},1.0);
        width = readFromTable(table, {"render", "width"},100);
        height = readFromTable(table, {"render", "height"},100);

        break;
    }
}

optional<toml::table> ConfigParser::readToml(std::string path) {
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
        return tbl;
    } catch (const toml::parse_error &err) {
        std::cerr << "Parsing of '" << path << "' failed:\n" << err << "\n";
    }
    return {};
}