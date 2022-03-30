cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(luacpp)

include(utility/utility.cmake)



add_library(libluacpp STATIC source/luacpp.cpp)
target_include_directories(libluacpp PUBLIC include PRIVATE source)
target_link_libraries(libluacpp PUBLIC liblua)

ADD_CMAKE_SUBDIRS_HERE()
