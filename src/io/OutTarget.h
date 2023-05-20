//
// Created by Burg on 19.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_OUTTARGET_H
#define NON_EUCLIDEAN_RENDER_OUTTARGET_H

#include <string>
#include <vector>
#include <mutex>
#include <SDL.h>
#include "interface/RenderingTarget.h"
#include "interface/LoggingTarget.h"

typedef unsigned char u8;

// suggestions for debug levels
enum DebugLevel {
    DEFAULT = 0,
    IMPORTANT = 10,
    MOST = 100,
    ALL = INT_MAX
};

struct FileOut : RenderingTarget, LoggingTarget {
    std::string path;
    std::string name;

    // only relevant if used as logging target
    bool disableDebug = false;      // disables all debugs
    int debugLevel = INT_MAX;       // only debugs on this level or lower will be executed
    bool threadSave;

    FileOut(const std::string &path, const std::string &name);

    void writeOut(std::pair<int, int> resolution, std::vector<u8> &data) override;

    void writeOut(std::string &data) override;

    void writeOutNewLine(std::string &data) override;


    void debug(std::string &data) override;

    void debug(std::string &data, int level) override;

    void debugNewLine(std::string &data) override;

    void debugNewLine(std::string &data, int level) override;

private:
    std::mutex consoleAccessMut;
};

struct ConsoleOut : LoggingTarget {
    bool autoFlush = true;          // automatically flushes console after message
    bool disableDebug = false;      // disables all debugs
    bool debugToStderr = false;     // will debug to cerr if enabled
    int debugLevel = INT_MAX;       // only debugs on this level or lower will be executed

    bool threadSave = false;        // if active will sync output between threads

    void writeOut(std::string &data) override;

    void writeOutNewLine(std::string &data) override;

    // debugs on level 0
    void debug(std::string &data) override;

    void debug(std::string &data, int level) override;

    // debugs on level 0
    void debugNewLine(std::string &data) override;

    void debugNewLine(std::string &data, int level) override;


private:
    std::mutex consoleAccessMut;
};

struct ScreenOut : RenderingTarget {
    SDL_Window* window;
    SDL_Renderer* sdlRenderer;
    SDL_Texture* buffer;

    ScreenOut(int width, int height, std::string title);
    void writeOut(std::pair<int, int> resolution, std::vector<unsigned char> &data);

    ~ScreenOut();
};

#endif //NON_EUCLIDEAN_RENDER_OUTTARGET_H
