//
// Created by Burg on 20.05.2023.
//

#include "RacingGame.h"
#include <iostream>
#include "../io/ConfigParser.h"
#include "../io/OBJ_parser.h"
#include "../renderer/GeoGraph.h"
#include "../renderer/Renderer.h"
#include "../io/KeyboardAdapter.h"
#include "glm/matrix.hpp"
using glm::mat2;
using namespace std;

RacingGame::RacingGame(Renderer &renderer, GeoGraph graph) : graph(graph), renderer(renderer){}

void RacingGame::run() {
    State state;
    state.tri = 0;
    state.pos = graph.triangles[state.tri].getMid();

    state.dir = glm::vec2(1,0);

    KeyboardAdapter keyboardAdapter;
    float turningAngle = 1.0f  / framerate;
    mat2 rotationMatR = {cos(turningAngle), sin(turningAngle), -sin(turningAngle), cos(turningAngle)};
    mat2 rotationMatL = {cos(-turningAngle), sin(-turningAngle), -sin(-turningAngle), cos(-turningAngle)};

    float speed = 1.0f / framerate;

    const int framMicro = 1e6 / framerate;
    auto t1 = chrono::high_resolution_clock::now();
    int count = 0;
    while(!keyboardAdapter.quit) {
        this_thread::sleep_for(chrono::microseconds(
                max((int)chrono::duration_cast<chrono::microseconds>(t1 - chrono::high_resolution_clock::now()).count() + framMicro,0)));
        t1 = chrono::high_resolution_clock::now();
        count ++;
        cerr << count << endl;
        if (keyboardAdapter.isDown(SDL_Scancode::SDL_SCANCODE_A))
            state.dir = rotationMatR * state.dir;
        if (keyboardAdapter.isDown(SDL_Scancode::SDL_SCANCODE_D))
            state.dir = rotationMatL * state.dir;

        state.dir = {-state.dir.y,state.dir.x};
        state = graph.traverse(state,speed).first;

        state.dir = {state.dir.y,-state.dir.x};
        renderer.render(state, 50);
    }

    renderer.close(true);
}
