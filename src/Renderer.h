//
// Created by Burg on 13.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_RENDERER_H
#define NON_EUCLIDEAN_RENDER_RENDERER_H

#endif //NON_EUCLIDEAN_RENDER_RENDERER_H

#include <utility>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>

#include "OBJ_parser.h"
#include "GeoGraph.h"

typedef unsigned char u8;

struct RenderingTarget {
    virtual void writeOut(pair<int, int> resolution, vector<u8> &data) = 0;
};

struct LoggingTarget {
    virtual void writeOut(vector<string> &data) = 0;

    virtual void writeOutNewLine(string &data) = 0;

    virtual void writeOut(string &data) = 0;
};

struct FileOut : RenderingTarget, LoggingTarget {
    string path;
    string name;

    FileOut(const string path, const string name) : path(path), name(name) {
        // todo: comment in
        //filesystem::create_directories(path);
        //ofstream file;
        //file.open(path + name);
        //file.close();
    }

    void writeOut(pair<int, int> resolution, vector<u8> &data) override;

    void writeOut(vector<string> &data) override;

    void writeOut(string &data) override;

    void writeOutNewLine(string &data) override;
};

struct ConsoleOut : LoggingTarget {
    void writeOut(vector<string> &data) override;

    void writeOut(string &data) override;

    void writeOutNewLine(string &data) override;
};

struct Ray {
    Vec2d direction;
    vector<pair<float, int>> renderPoints;
};

struct RenderBuffer {
private:
    vector<bool> pixelSet;
    int count = 0;
    bool full = false;
    mutex fullMut;
    mutex countMut;

public:
    vector<u8> pixel;
    condition_variable condVar;
    const int frame;

    RenderBuffer(const int frame, int size) : frame(frame) {

        pixel = vector<u8>(size,48);
        fullMut.lock();
    }

    void notifyCount(int add){
        countMut.lock();
        count += add;
        countMut.unlock();
        // cout << "- data: " << count << " of " << pixel.size() << "  pos: " << start << "\n";
        if (count == pixel.size()) {
            // cout << "- data full" << endl;
            fullMut.unlock();
        }
    }

    vector<u8> getData() {
        return pixel;
    }

    void waitFull() {
        fullMut.lock();
        // cout << "### is full" << endl;
        fullMut.unlock();
    }

};

struct RenderChunk {
    int frame;
    RenderBuffer* rb;
    State start;
    double scale;
    vector<Ray *> *rays;
};

struct ThreadContext {
    int width;
    int height;

    GeoGraph graph;

    mutex renderQueueMutex;
    queue<RenderChunk> renderQueue;

    condition_variable newDataCond;
    mutex newDataCondMutex;
    deque<RenderBuffer*> renderBuffers;

    ThreadContext(int width, int height, const GeoGraph graph) : width(width), height(height),
                                                                 graph(graph) {


    };

};

struct Renderer {
    const static int pixelPerChunk = 10000;
    const static int renderThreadAmount = 10;
    int frameCount = 0;
    int w, h;
    RenderingTarget &renderingTarget;
    vector<Ray> top;
    vector<Ray> bottom;
    vector<Ray> left;
    vector<Ray> right;

    vector<vector<Ray *>> chunks;

    thread mainThread;
    vector<thread> renderThreads;

    ThreadContext threadContext;

    Renderer(int w, int h, GeoGraph graph, RenderingTarget &target);

    void
    renderDebug(State start, double scale, const vector<LoggingTarget *> &loggingTargets, map<string, string> info);

    void render(State startState, double scale, const vector<LoggingTarget *> &dataAccesMutex = {});
};



