cmake_minimum_required(VERSION 3.25)
project(non_euclidean_render)

set(CMAKE_CXX_STANDARD 17)

add_executable(non_euclidean_render 
    main.cpp
    stbi_impl.cpp
        src/ConfigParser.cpp src/ConfigParser.h src/OBJ_parser.cpp src/OBJ_parser.h)