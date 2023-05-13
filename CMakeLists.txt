cmake_minimum_required(VERSION 3.18)
project(non_euclidean_render)

set(CMAKE_CXX_STANDARD 17)

add_executable(non_euclidean_render 
    main.cpp
    stbi_impl.cpp
        src/ConfigParser.cpp src/ConfigParser.h src/OBJ_parser.cpp src/OBJ_parser.h src/GeoGraph.cpp src/MathUtil.cpp src/MathUtil.h src/GeoGraph.h src/Renderer.h src/Renderer.cpp)