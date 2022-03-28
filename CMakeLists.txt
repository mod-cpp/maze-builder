cmake_minimum_required(VERSION 3.20)

include(cmake/vcpkg.cmake)

project(maze-builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

enable_testing()

add_subdirectory(src)
