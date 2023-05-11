//
// Created by Burg on 11.05.2023.
//

#ifndef NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
#define NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
#include <iostream>
#include <filesystem>

struct OBJ_parser {
    std::vector<std::tuple<float,float,float>> vertices;
    std::vector<std::tuple<float,float,float>> normals;
    std::vector<std::tuple<float,float>> textureCords;

    std::vector<std::tuple<int,int,int>> triangle_vertices;
    std::vector<std::tuple<int,int,int>> triangle_normals;
    std::vector<std::tuple<int,int,int>> triangle_textureCords;

public:
    void loadFromFile(std::string path);


private:
    std::vector<std::string> getParts(std::string str, char delim);
};


#endif //NON_EUCLIDEAN_RENDER_OBJ_PARSER_H
