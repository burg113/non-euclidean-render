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
#include <vector>
#include <condition_variable>

#include "../io/OBJ_parser.h"
#include "GeoGraph.h"
#include "../io/OutTarget.h"

typedef unsigned char u8;

struct Ray {
    glm::vec2 direction;
    std::vector<std::pair<float, int>> renderPoints;
};

struct RenderBuffer {
private:
    std::vector<bool> pixelSet;
    int count = 0;
    std::mutex countMut;

    bool full = false;
    std::mutex fullMut;
    std::condition_variable fullCond;

public:
    std::vector<u8> pixel;
    const int frame;

    RenderBuffer(int frame, int size);

    void notifyCount(int add);

    void waitFull();

};

struct RenderChunk {
    int frame;
    RenderBuffer *rb;
    State start;
    double scale;
    std::vector<Ray *> *rays;

    RenderChunk(int frame, RenderBuffer *rb, const State &start, double scale, std::vector<Ray *> *rays);

};

struct ThreadContext {
    int width;
    int height;

    GeoGraph graph;

    std::mutex renderQueueMutex;
    std::queue<RenderChunk> renderQueue;

    std::condition_variable newDataCond;
    std::mutex newDataCondMutex;
    std::deque<RenderBuffer *> renderBuffers;

    std::vector<LoggingTarget *> loggingTargets;

    void log(std::string str);

    void debug(std::string str);

    void debug(std::string str, int level);

    ThreadContext(int width, int height, GeoGraph graph);

};

struct Renderer {
    const static int pixelPerChunk = 10000;
    const static int renderThreadAmount = 10;

    int frameCount = 0;
    int w, h;

    RenderingTarget &renderingTarget;
    std::vector<Ray> top;
    std::vector<Ray> bottom;
    std::vector<Ray> left;
    std::vector<Ray> right;

    std::vector<std::vector<Ray *>> chunks;

    std::thread mainThread;
    std::vector<std::thread> renderThreads;

    ThreadContext threadContext;

    Renderer(int w, int h, GeoGraph graph, RenderingTarget &target);

    void render(State startState, double scale);

    void addLoggingTarget(LoggingTarget *target);
};

