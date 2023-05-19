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

#include "../io/OBJ_parser.h"
#include "GeoGraph.h"
#include "../io/OutTarget.h"

typedef unsigned char u8;


struct Ray {
    Vec2d direction;
    vector<pair<float, int>> renderPoints;
};

struct RenderBuffer {
private:
    vector<bool> pixelSet;
    int count = 0;
    mutex countMut;

    bool full = false;
    mutex fullMut;
    condition_variable fullCond;

public:
    vector<u8> pixel;
    const int frame;

    RenderBuffer(const int frame, int size) : frame(frame) {
        pixel = vector<u8>(size, 48);
    }

    void notifyCount(int add) {
        countMut.lock();
        count += add;
        if (count == pixel.size()) {
            fullCond.notify_all();
            full = true;
        }
        countMut.unlock();
    }

    void waitFull() {
        unique_lock<mutex> lock(fullMut, defer_lock_t());
        lock.lock();
        if (!full)
            fullCond.wait(lock);
        lock.unlock();
    }

};

struct RenderChunk {
    int frame;
    RenderBuffer *rb;
    State start;
    double scale;
    vector<Ray *> *rays;

    RenderChunk(int frame, RenderBuffer *rb, const State &start, double scale, vector<Ray *> *rays): frame(
            frame), rb(rb), start(start), scale(scale), rays(rays) {};

};

struct ThreadContext {
    int width;
    int height;

    GeoGraph graph;

    mutex renderQueueMutex;
    queue<RenderChunk> renderQueue;

    condition_variable newDataCond;
    mutex newDataCondMutex;
    deque<RenderBuffer *> renderBuffers;

    vector<LoggingTarget *> loggingTargets;

    void log(string str) {
        for (LoggingTarget *target: loggingTargets)
            target->writeOutNewLine(str);
    };

    void debug(string str) {
        for (LoggingTarget *target: loggingTargets)
            target->debugNewLine(str);
    };

    void debug(string str, int level) {
        for (LoggingTarget *target: loggingTargets)
            target->debugNewLine(str, level);
    };


    ThreadContext(int width, int height, const GeoGraph graph) : width(width), height(height),
                                                                 graph(graph) {};

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

    void render(State startState, double scale);

    void addLoggingTarget(LoggingTarget *target);
};



