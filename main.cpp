#include <iostream>
#include "lib/toml.hpp"
#include <filesystem>

const char pathSeperator =
#ifdef _WIN32
        '\\';
#else
'/';
#endif



using namespace std;

struct Rect {
    int x, y, w, h;

    Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

    friend std::ostream& operator<< (std::ostream& stream, const Rect& rect) {
        return stream << "Rect: (" << rect.x << "," << rect.y <<")  dim: ("  << rect.w << "," << rect.h << ")";
    }
};


struct Circle {
    int x, y, r;

    Circle(int x, int y, int r) : x(x), y(y), r(r) {}

    friend std::ostream& operator<< (std::ostream& stream, const Circle& circle) {
        return stream << "Circle: (" << circle.x << "," << circle.y <<")  radius: "  << circle.r;
    }
};


string meshPath;
vector<Circle> circles;
vector<Rect> rects;


vector<tuple<float,float,float>> verticies;
vector<tuple<int,int,int>> triangle_verts;
vector<tuple<int,int,int>> triangle_normals;

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

void create(string name, vector<int> vars) {
    if (name == "primitive:rectangle" && vars.size() == 4)
        rects.emplace_back(vars[0],vars[1],vars[2],vars[3]);
    else if (name == "primitive:circle" && vars.size() == 3)
        circles.emplace_back(vars[0],vars[1],vars[2]);
}

void loadObject(toml::table &table) {
    auto O_name = table["name"].value<string>();
    auto vars = *table.get_as<toml::array>("vars");
    auto O_col = *table.get_as<toml::array>("col"); // ignored for now

    if (!O_name.has_value()) {
        cout << "invalid object: table" << "\n";
        return;
    }
    vector<int> params;
    int i = 0;
    while (vars.get(i) != nullptr) {
        auto a = (*vars.get(i)).value<int>();
        if (a.has_value()) {
            params.push_back(a.value());
        }
        i++;
    }

    string name = O_name.value();

    create(name,params);
    cout << "parsed:" << table << "\n";
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
            cout << "error: missing mesh file\n";
            continue;
        }

        meshPath = input.substr(0, input.find_last_of(pathSeperator) + 1) + s.value();

        toml::array &objects = *(*table.get_as<toml::table>("scene")).get_as<toml::array>("objects");

        for (auto &&elem: objects) {
            elem.visit([](auto &&el) noexcept {
                if constexpr (toml::is_table<decltype(el)>)
                    loadObject(el);
                else
                    cout << "ignoring:" << el << "\n";
            });

        }
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
            triangle_verts.emplace_back(nums[0],nums[3],nums[6]);
            triangle_normals.emplace_back(nums[2],nums[5],nums[8]);

            continue;
        } else if (line[1] == ' ') { // vertex
            vector<double> nums;
            for(const string& val : getParts(line.substr(1,line.size()-1),' ')) {
                nums.push_back(stod(val));
            }
            if (nums.size() != 3)
                throw invalid_argument("invalid mesh file 2");
            verticies.emplace_back(nums[0],nums[1],nums[2]);
        }else if (line[1] == 'n') { // normals
            vector<double> nums;
            for(const string& val : getParts(line.substr(2,line.size()-2),' ')) {
                nums.push_back(stod(val));
            }
            if (nums.size() != 3)
                throw invalid_argument("invalid mesh file 2");
            verticies.emplace_back(nums[0],nums[1],nums[2]);
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