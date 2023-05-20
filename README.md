# Non Euclidean Render

This projekt is focused on creating a 2D renderer for non euklidian geometry. At some point in time it is supposed to be turned into a gameplay demo of sorts.

For now it is more of an abstract art generator :

![not available](https://github.com/burg113/non-euclidean-render/blob/master/results/cube.png)
![not available](https://github.com/burg113/non-euclidean-render/blob/master/results/sphere.jpg)
![not available](https://github.com/burg113/non-euclidean-render/blob/master/results/donut_high_res.png)


## Building
This project needs SDL2 as a dependency. [GitHub](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.5). (Use a package manager on Linux.)
If you are on Windows, you need to tell CMAKE where to find the package. Put this into CMakeLists.txt
```
list(APPEND CMAKE_PREFIX_PATH path/to/library/folder) # or use ${CMAKE_CURRENT_SOURCE_DIR} for relative path
```
After using CMAKE to configure the project, the location will be cached, so you can delete the line again.
Then, you also need to move the .dll in the same folder as the binary.