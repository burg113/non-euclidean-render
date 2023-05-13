//
// Created by Burg on 13.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_RENDERER_H
#define NON_EUCLIDEAN_RENDER_RENDERER_H

#endif //NON_EUCLIDEAN_RENDER_RENDERER_H

#include <utility>

#include "OBJ_parser.h"
#include "GeoGraph.h"

typedef unsigned char u8;

struct RenderingTarget {
    virtual void writeOut(pair<int, int> resolution, vector<u8> &data) = 0;
};

struct LoggingTarget {
    virtual void writeOut(vector<string> &data) = 0;
    virtual void writeOutNewLine(string& data) = 0;
    virtual void writeOut(string &data) = 0;
};

struct FileOut : RenderingTarget, LoggingTarget {
    string path;
    string name;

    FileOut(const string &path, const string &name) : path(path), name(name) {
        filesystem::create_directories(path);
        ofstream file;
        file.open(path + name + ".log");
        file.close();
    }

    void writeOut(pair<int, int> resolution, vector<u8> &data) override;

    void writeOut(vector<string> &data) override;
    void writeOut(string& data) override;
    void writeOutNewLine(string& data) override;
};

struct ConsoleOut: LoggingTarget {
    void writeOut(vector<string> &data) override;
    void writeOut(string &data) override;
    void writeOutNewLine(string& data) override;
};

struct Renderer {
    int w, h;
    GeoGraph graph;
    RenderingTarget &renderingTarget;


    Renderer(int w, int h, GeoGraph graph, RenderingTarget &target) : w(w), h(h), graph(std::move(graph)), renderingTarget(target) {}

    void debugRender(State start, int scale, const vector<LoggingTarget*>& loggingTargets, map<string,string> info);

    void render(State start, int scale,const vector<LoggingTarget *> &loggingTargets = {});
};