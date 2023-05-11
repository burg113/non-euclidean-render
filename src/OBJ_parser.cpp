//
// Created by Burg on 11.05.2023.
//
#include "OBJ_parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;

void OBJ_parser::loadFromFile(std::string path) {
    ifstream file (path);
    if (!file.is_open()) {
        cout << "error: could not open mesh at" << path << "\nunrecoverable";
        return;
    }

    string line;
    while (getline(file,line)) {
        if (line.size() < 3 || line[0] != 'v' && line[0] != 'f')
            continue;
        if (line[0] == 'f') { // triangle
            vector<int> nums;
            for (const string &str: getParts(line.substr(1, line.size() - 1), ' ')) {
                for (const string &val: getParts(str, '/')) {
                    nums.push_back(stoi(val));
                }
            }
            if (nums.size() != 9)
                throw invalid_argument("cannot deal with this format: " + line);
            triangle_vertices.emplace_back(nums[0], nums[3], nums[6]);
            triangle_normals.emplace_back(nums[2], nums[5], nums[8]);

            continue;
        } else if (line[1] == ' ') { // vertex
            vector<double> nums;
            for (const string &val: getParts(line.substr(1, line.size() - 1), ' ')) {
                nums.push_back(stod(val));
            }
            if (nums.size() != 3)
                throw invalid_argument("invalid mesh file 2");
            vertices.emplace_back(nums[0], nums[1], nums[2]);
        } else if (line[1] == 'n') { // normals
            vector<double> nums;
            for (const string &val: getParts(line.substr(2, line.size() - 2), ' ')) {
                nums.push_back(stod(val));
            }
            if (nums.size() != 3)
                throw invalid_argument("invalid mesh file 2");
            normals.emplace_back(nums[0], nums[1], nums[2]);
        }
    }
}

vector<string> OBJ_parser::getParts(string str, char delim) {
    vector<string> v;
    string current = "";
    for (char c : str) {
        if (c== delim) {
            if (!current.empty())
                v.push_back(current);
            current = "";
        }else
            current+= c;
    }
    if (!current.empty())
        v.push_back(current);
    return v;
}