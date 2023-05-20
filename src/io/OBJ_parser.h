//
// Created by Burg on 11.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
#define NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
#include <iostream>
#include <filesystem>
#include <regex>
#include "../util/MathUtil.h"

struct OBJ_parser {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;

    std::vector<std::tuple<int,int,int>> triangleVertices;
    std::vector<std::tuple<int,int,int>> triangleNormals;
    std::vector<std::tuple<int,int,int>> triangleUV;

    void loadFromFile(std::string path) noexcept(false);


private:
    static const std::regex vertexRegex;
    static const std::regex normalRegex;
    static const std::regex textureRegex;
    static const std::regex triangleRegex;
};


#endif //NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
