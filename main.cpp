#include <iostream>
#include "lib/toml.hpp"
#include <filesystem>

const char pathSeperator = '/';

using namespace std;

string meshPath;


vector<tuple<float,float,float>> vertices;
vector<tuple<float,float,float>> normals;
vector<tuple<float,float>> textureCords;

vector<tuple<int,int,int>> triangle_vertices;
vector<tuple<int,int,int>> triangle_normals;
vector<tuple<int,int,int>> triangle_textureCords;

optional<toml::table> readToml(std::string path) {
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
        return tbl;
    } catch (const toml::parse_error &err) {
        std::cerr << "Parsing of '" << path << "' failed:\n" << err << "\n";
    }
    return {};
}

void loadConfig(string tryFirst) {
    while (true) {
        string input = tryFirst;
        if (!tryFirst.empty()) {
            input = tryFirst;
            tryFirst = "";
        } else {
            cout << "please input the path to the scene file: \n";
            getline(cin,input);
        }

        optional<toml::table> t = readToml(input);
        if (!t.has_value())
            continue;
        toml::table table = t.value();

        // read in a valid toml
        auto s = table["mesh"]["file"].value<string>();
        if (!s.has_value()) {
            cout << "error: missing mesh path\n";
            continue;
        }
        meshPath = input.substr(0, input.find_last_of(pathSeperator) + 1) + s.value();
        break;
    }
}


vector<string> getParts(string str, char delim) {
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

void loadMesh(string path) {
    ifstream file (path);
    if (!file.is_open()) {
        cout << "error: could not open mesh at" << path << "\nunrecoverable";
        return;
    }

    string line;
    while (getline(file,line)) {
        if(line.size() < 3 || line[0] != 'v' && line[0] != 'f')
            continue;
        if (line[0] == 'f') { // triangle
            vector<int> nums;
            for (const string& str : getParts(line.substr(1,line.size()-1),' ')) {
                for(const string& val : getParts(str,'/')) {
                    nums.push_back(stoi(val));
                }
            }
            if (nums.size() != 9)
                throw invalid_argument("cannot deal with this format: " + line);
            triangle_vertices.emplace_back(nums[0],nums[3],nums[6]);
            triangle_normals.emplace_back(nums[2],nums[5],nums[8]);

            continue;
        } else if (line[1] == ' ') { // vertex
            vector<double> nums;
            for(const string& val : getParts(line.substr(1,line.size()-1),' ')) {
                nums.push_back(stod(val));
            }
            if (nums.size() != 3)
                throw invalid_argument("invalid mesh file 2");
            triangle_vertices.emplace_back(nums[0],nums[1],nums[2]);
        }else if (line[1] == 'n') { // normals
            vector<double> nums;
            for(const string& val : getParts(line.substr(2,line.size()-2),' ')) {
                nums.push_back(stod(val));
            }
            if (nums.size() != 3)
                throw invalid_argument("invalid mesh file 2");
            triangle_vertices.emplace_back(nums[0],nums[1],nums[2]);
        }
    }
}


int main(int argc, char **argv) {

    string fileDir;
    if (argc > 1)
        fileDir = argv[1];

    loadConfig(fileDir);

    loadMesh(meshPath);

    /*for (auto a : verticies) {
        cout << get<0>(a) << " " << get<1>(a) << " " << get<2>(a) << "\n";
    }*/

    return 0;

}