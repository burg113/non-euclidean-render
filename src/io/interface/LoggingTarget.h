//
// Created by Burg on 19.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_LOGGINGTARGET_H
#define NON_EUCLIDEAN_RENDER_LOGGINGTARGET_H

#include <vector>
#include <string>

struct LoggingTarget {

    virtual void writeOut(std::string &data) = 0;
    virtual void writeOutNewLine(std::string &data) = 0;

    virtual void debug(std::string &data) = 0;
    virtual void debug(std::string &data, int level) = 0;

    virtual void debugNewLine(std::string &data) = 0;
    virtual void debugNewLine(std::string &data, int level) = 0;
};

#endif //NON_EUCLIDEAN_RENDER_LOGGINGTARGET_H
