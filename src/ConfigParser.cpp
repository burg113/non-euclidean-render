//
// Created by Burg on 11.05.2023.
//
#include <iostream>
#include "../lib/toml.hpp"

using namespace std;

// read string from given toml table
string readString(toml::table &table, vector<string> path,string errorMsg = "", string stdReturn="") {
    toml::node_view a = table[path[0]];
    for (int i = 0; i < path.size() - 1; i++)
        a = a[path[i + 1]];
    auto s = a.value<string>();
    if (!s.has_value()) {
        if (errorMsg.size()>0)
            cout << errorMsg << "\n";
        return stdReturn;
    }else
        return s.value();
}

#include "ConfigParser.h"

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

        meshPath = basePath + readString(table,{"mesh","file"},"error: missing mesh path");
        outPath = basePath + readString(table,{"output","path"});
        outFileName = readString(table,{"output","filename"});

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