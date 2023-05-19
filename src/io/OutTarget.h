//
// Created by Burg on 19.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_OUTTARGET_H
#define NON_EUCLIDEAN_RENDER_OUTTARGET_H

#include <string>
#include <vector>
#include "interface/RenderingTarget.h"
#include "interface/LoggingTarget.h"
typedef unsigned char u8;

struct FileOut : RenderingTarget, LoggingTarget {
    std::string path;
    std::string name;

    FileOut(const std::string& path, const std::string& name);

    void writeOut(std::pair<int, int> resolution, std::vector<u8> &data) override;

    void writeOut(std::vector<std::string> &data) override;

    void writeOut(std::string &data) override;

    void writeOutNewLine(std::string &data) override;
};

struct ConsoleOut : LoggingTarget {
    void writeOut(std::vector<std::string> &data) override;

    void writeOut(std::string &data) override;

    void writeOutNewLine(std::string &data) override;
};

struct ScreenOut : RenderingTarget {
    void writeOut(std::pair<int, int> resolution, std::vector<unsigned char> &data);
};

#endif //NON_EUCLIDEAN_RENDER_OUTTARGET_H
