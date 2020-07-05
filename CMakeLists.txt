cmake_minimum_required (VERSION 3.0.2)

project(gpark)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

set(CMAKE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(CMAKE_THIRDLIB_ROOTDIR ${CMAKE_ROOT_DIR}/3rdlib)

include(${CMAKE_ROOT_DIR}/BuildScript/Build.cmake)

# enable project folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

############

add_subdirectory("gpark")

