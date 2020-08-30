project(stalk-api)

cmake_minimum_required(VERSION 3.4)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

include(CMakeIncludes.txt)

add_subdirectory(external/LUrlParser)
add_subdirectory(external/logger)
add_subdirectory(external/prometheus-cpp)
add_subdirectory(src)

