//
// Created by Burg on 19.05.2023.
//

#include "OutTarget.h"
#include <fstream>
#include <utility>
#include <filesystem>
#include <iostream>
#include "../../lib/stb_image_write.h"

using namespace std;


FileOut::FileOut(const string &path, const string &name) : path(path), name(name) {
    // comment out for disabling writing to file:
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name);
    file.close();
}

void FileOut::writeOut(pair<int, int> resolution, vector<u8> &data) {
    // cout << "writing to " << path << endl;
    filesystem::create_directories(path);
    stbi_write_png((path + name).c_str(), resolution.first, resolution.second, 3, data.data(), 0);
}

void FileOut::writeOut(std::string &str) {
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    file << str;
    file.close();
}

void FileOut::writeOutNewLine(std::string &str) {
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    file << str << "\n";
    file.close();
}

void FileOut::writeOut(vector<std::string> &data) {
    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    for (const string &str: data) {
        file << str << "\n";
    }
    file.close();
}


void ConsoleOut::writeOut(vector<string> &data) {
    for (const auto &str: data)
        cout << str << "\n";
}

void ConsoleOut::writeOut(string &str) {
    cout << str;
}

void ConsoleOut::writeOutNewLine(std::string &str) {
    cout << str << "\n";
}