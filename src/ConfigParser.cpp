//
// Created by Burg on 11.05.2023.
//
#include <iostream>
#include "../lib/toml.hpp"
using namespace std;

#include "ConfigParser.h"
void ConfigParser::loadConfig(string tryFirst) {
    while (true) {
        string input = tryFirst;
        if (!tryFirst.empty()) {
            input = tryFirst;
            tryFirst = "";
        } else {
            cout << "please input the path to the scene file: \n";
            getline(cin,input);
        }

        optional<toml::table> t = readToml(input);
        if (!t.has_value())
            continue;
        toml::table table = t.value();

        // read in a valid toml
        auto s = table["mesh"]["file"].value<string>();
        if (!s.has_value()) {
            cout << "error: missing mesh path\n";
            continue;
        }
        meshPath = input.substr(0, input.find_last_of(pathSeparator) + 1) + s.value();
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