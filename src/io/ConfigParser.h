//
// Created by Burg on 11.05.2023.
//
#include <iostream>
#include "toml.hpp"

#ifndef NON_EUCLIDEAN_RENDER_TOMLPARSER_H
#define NON_EUCLIDEAN_RENDER_TOMLPARSER_H


struct ConfigParser {
    const char pathSeparator = '/';
    std::string meshPath;
    std::string texturePath;
    std::string outPath;
    std::string outFileName;
    int width;
    int height;
    double scale;

    void loadConfig(std::string tryFirst);

private:
    std::optional<toml::table> readToml(std::string path);
};


#endif //NON_EUCLIDEAN_RENDER_TOMLPARSER_H
