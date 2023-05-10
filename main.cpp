#include <iostream>
#include "lib/toml.hpp"


using namespace std;

toml::table readToml(std::string path) {
    toml::table tbl;
    try
    {
        tbl = toml::parse_file(path);
        std::cout << tbl << "\n";
    }
    catch (const toml::parse_error& err)
    {
        std::cerr << "Parsing failed:\n" << err << "\n";
    }
    return tbl;
}



int main(int argc, char** argv) {
    string fileDir;
    if (argc == 0) {
        cout << "please input the path to the scene file: \n" << endl;
        cin >> fileDir;
    }else
        fileDir = argv[1];


    toml::table t = readToml(fileDir);




    return 0;
}
