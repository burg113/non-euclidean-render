//
// Created by Burg on 11.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
#define NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
#include <iostream>
#include <filesystem>
#include <regex>
#include "MathUtil.h"

struct OBJ_parser {
    std::vector<Vec3d> vertices;
    std::vector<Vec3d> normals;
    std::vector<Vec2d> textureCords;

    std::vector<std::tuple<int,int,int>> triangle_vertices;
    std::vector<std::tuple<int,int,int>> triangle_normals;
    std::vector<std::tuple<int,int,int>> triangle_textureCords;

    void loadFromFile(std::string path) noexcept(false);


private:
    static const std::regex vertexRegex;
    static const std::regex normalRegex;
    static const std::regex textureRegex;
    static const std::regex triangleRegex;
};


#endif //NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
