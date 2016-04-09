cmake_minimum_required(VERSION 2.8)
project(CRout)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES crout.hpp grid.hpp matrix.hpp Time.hpp crout.cpp convolution.cpp initiate.cpp unit_hydros.cpp write_file.cpp)

add_executable(crout ${SOURCE_FILES})
