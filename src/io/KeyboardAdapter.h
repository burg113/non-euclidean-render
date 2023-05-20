//
// Created by Burg on 20.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_KEYBOARDADAPTER_H
#define NON_EUCLIDEAN_RENDER_KEYBOARDADAPTER_H

#include <SDL.h>
#include <map>
#include <vector>

// should be unique
struct KeyboardAdapter {

    std::vector<bool> keys;
    bool quit = false;

    KeyboardAdapter();

    // updates the state
    void update();

    // updates the state and returns true if the given key is down
    bool isDown(SDL_Scancode keyCode);

};


#endif //NON_EUCLIDEAN_RENDER_KEYBOARDADAPTER_H
