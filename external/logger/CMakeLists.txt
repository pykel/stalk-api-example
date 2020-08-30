project(logger)
set(PROJECT_VERSION 0.1)

cmake_minimum_required(VERSION 3.4)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

include_directories(${SPDLOG_INCLUDEDIR})

set(EXTERNAL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/logger)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
    )

add_library(${PROJECT_NAME} STATIC
            ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1)

target_link_libraries(${PROJECT_NAME}
)

