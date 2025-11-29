cmake_minimum_required(VERSION 3.16)

project(VoxelRaytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/raylib-master)

add_executable(VoxelRaytracer
    src/main.cpp
    src/voxel.cpp
    src/raytracer.cpp
 "src/constants.h")

target_link_libraries(VoxelRaytracer PRIVATE raylib)