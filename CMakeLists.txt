cmake_minimum_required(VERSION 3.10)

project(plonk VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include(CTest)

add_subdirectory(app)
add_subdirectory(libs)
add_subdirectory(libs/vendor/glfw)
