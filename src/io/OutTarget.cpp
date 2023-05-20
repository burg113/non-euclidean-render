//
// Created by Burg on 19.05.2023.
//

#include "OutTarget.h"
#include <fstream>
#include <utility>
#include <filesystem>
#include <iostream>
#include "stb_image_write.h"

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


ScreenOut::ScreenOut(int width, int height, std::string title){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "SDL could not be initialized!" << std::endl
                  << "SDL_Error: " << SDL_GetError() << std::endl;
        throw invalid_argument("SDL init error");
    }
    window = SDL_CreateWindow(title.c_str(),
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          width, height,
                                          SDL_WINDOW_SHOWN);
    if(!window){
        std::cout << "Window could not be created!" << std::endl
                  << "SDL_Error: " << SDL_GetError() << std::endl;
        throw invalid_argument("SDL window init error");
    }
    sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    buffer = SDL_CreateTexture(sdlRenderer,
                               SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               width,
                               height);
}

void ScreenOut::writeOut(std::pair<int, int> resolution, vector<unsigned char> &data) {
    SDL_RenderClear(sdlRenderer);
    void* pixels;
    int pitch;
    SDL_LockTexture(buffer, NULL, &pixels, &pitch);
    memcpy(pixels, data.data(), pitch*resolution.second);
    SDL_UnlockTexture(buffer);
    SDL_RenderCopy(sdlRenderer, buffer, NULL, NULL);
    SDL_RenderPresent(sdlRenderer);
}

ScreenOut::~ScreenOut() {
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
}
