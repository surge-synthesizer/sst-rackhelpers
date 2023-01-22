cmake_minimum_required(VERSION 3.10)
project(sst-rackhelpers VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
