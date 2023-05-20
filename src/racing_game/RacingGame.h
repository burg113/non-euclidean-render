//
// Created by Burg on 20.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_RACINGGAME_H
#define NON_EUCLIDEAN_RENDER_RACINGGAME_H


#include "../io/ConfigParser.h"
#include "../renderer/GeoGraph.h"
#include "../renderer/Renderer.h"

struct RacingGame {
    GeoGraph graph;
    Renderer &renderer;
    int framerate = 200;

    RacingGame(Renderer &renderer, GeoGraph graph);

    void run();

};


#endif //NON_EUCLIDEAN_RENDER_RACINGGAME_H
