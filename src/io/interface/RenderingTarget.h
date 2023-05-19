//
// Created by Burg on 19.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_RENDERINGTARGET_H
#define NON_EUCLIDEAN_RENDER_RENDERINGTARGET_H

#include <utility>
#include <vector>

struct RenderingTarget {
    virtual void writeOut(std::pair<int, int> resolution, std::vector<unsigned char> &data) = 0;
};


#endif //NON_EUCLIDEAN_RENDER_RENDERINGTARGET_H
