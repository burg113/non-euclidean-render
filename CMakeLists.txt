cmake_minimum_required(VERSION 3.20)
project(non_euclidean_render)

set(CMAKE_CXX_STANDARD 17)

add_executable(non_euclidean_render
        main.cpp
        lib/single_header/implementation.cpp

        src/renderer/GeoGraph.h
        src/renderer/GeoGraph.cpp
        src/renderer/Renderer.h
        src/renderer/Renderer.cpp

        src/io/ConfigParser.cpp
        src/io/ConfigParser.h
        src/io/OBJ_parser.cpp
        src/io/OBJ_parser.h
        src/io/OutTarget.cpp
        src/io/OutTarget.h

        src/io/interface/LoggingTarget.h
        src/io/interface/RenderingTarget.h

        src/util/MathUtil.cpp
        src/util/MathUtil.h
        )

include_directories(
        lib/single_header/
        lib/glm-0.9.9.8/
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(non_euclidean_render ${SDL2_LIBRARIES})