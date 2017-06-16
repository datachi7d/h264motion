cmake_minimum_required (VERSION 2.8.12.2)

project (h264motion_lib)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)
find_package (FFmpeg REQUIRED)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)


add_subdirectory (lib)
add_subdirectory (src)
