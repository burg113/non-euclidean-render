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
    bool setMut = threadSave;
    if (setMut) consoleAccessMut.lock();

    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    file << str;
    file.close();

    if (setMut) consoleAccessMut.unlock();
}

void FileOut::writeOutNewLine(std::string &str) {
    bool setMut = threadSave;
    if (setMut) consoleAccessMut.lock();

    filesystem::create_directories(path);
    ofstream file;
    file.open(path + name, ios_base::app);
    file << str << "\n";
    file.close();

    if (setMut) consoleAccessMut.unlock();
}

void FileOut::debug(string &data) {
    debug(data,0);
}

void FileOut::debugNewLine(string &data) {
    debugNewLine(data,0);
}

void FileOut::debugNewLine(string &data, int level) {
    if (!disableDebug&& level <= debugLevel) writeOutNewLine(data);
}

void FileOut::debug(string &data, int level) {
    if (!disableDebug&& level <= debugLevel) writeOut(data);
}


void ConsoleOut::writeOut(string &data) {
    bool setMut = threadSave;
    if (setMut) consoleAccessMut.lock();

    cout << data;
    if (autoFlush) cout.flush();

    if (setMut) consoleAccessMut.unlock();
}

void ConsoleOut::writeOutNewLine(std::string &data) {
    bool setMut = threadSave;
    if (setMut) consoleAccessMut.lock();

    cout << data << "\n";
    if (autoFlush) cout.flush();

    if (setMut) consoleAccessMut.unlock();
}

void ConsoleOut::debug(string &data) {
    debug(data, 0);
}

void ConsoleOut::debugNewLine(string &data) {
    debugNewLine(data, 0);
}

void ConsoleOut::debug(string &data, int level) {
    bool setMut = threadSave;
    if (setMut) consoleAccessMut.lock();

    if (!disableDebug && level <= debugLevel) {
        if (debugToStderr) {
            cerr << data;
            if (autoFlush) cerr.flush();
        } else {
            cout << data;
            if (autoFlush) cout.flush();
        }
    }

    if (setMut) consoleAccessMut.unlock();
}

void ConsoleOut::debugNewLine(string &data, int level) {
    bool setMut = threadSave;
    if (setMut) consoleAccessMut.lock();

    if (!disableDebug && level <= debugLevel) {
        if (debugToStderr) {
            cerr << data << "\n";
            if (autoFlush) cerr.flush();
        }else {
            cout << data << "\n";
            if (autoFlush) cout.flush();
        }

    }

    if (setMut) consoleAccessMut.unlock();
}
