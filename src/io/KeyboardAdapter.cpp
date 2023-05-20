//
// Created by Burg on 20.05.2023.
//

#include <iostream>
#include "KeyboardAdapter.h"

KeyboardAdapter::KeyboardAdapter() {
    keys = std::vector<bool>(SDL_Scancode::SDL_NUM_SCANCODES);
}


void KeyboardAdapter::update() {
    SDL_Event event;
    while(SDL_PollEvent(&event) ) {
        if(event.type == SDL_KEYDOWN)
            keys[event.key.keysym.scancode] = true;
        if(event.type == SDL_KEYUP)
            keys[event.key.keysym.scancode] = false;
        if(event.type == SDL_QUIT)
            quit = true;
    }


}

bool KeyboardAdapter::isDown(SDL_Scancode keyCode) {
    update();
    return keys[keyCode];
}

