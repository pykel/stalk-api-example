project(prometheus-cpp)
set(PROJECT_VERSION 1.0)

cmake_minimum_required(VERSION 3.4)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

find_package(Threads REQUIRED)

find_package(PkgConfig)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/prometheus-cpp/core/include/)

set(S "${CMAKE_CURRENT_SOURCE_DIR}/prometheus-cpp/core")
set(SOURCE_FILES
  ${S}/src/check_names.cc
  ${S}/src/counter.cc
  ${S}/src/detail/builder.cc
  ${S}/src/detail/ckms_quantiles.cc
  ${S}/src/detail/time_window_quantiles.cc
  ${S}/src/detail/utils.cc
  ${S}/src/family.cc
  ${S}/src/gauge.cc
  ${S}/src/histogram.cc
  ${S}/src/registry.cc
  ${S}/src/serializer.cc
  ${S}/src/summary.cc
  ${S}/src/text_serializer.cc
)

add_library(${PROJECT_NAME} STATIC
            ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${main_lib_dest}")
INSTALL(FILES ${HEADER_FILES} DESTINATION "${include_dest}/prometheus-cpp")

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(INCLUDE_PATH "/${PROJECT_NAME}-${PROJECT_VERSION}")
foreach(LIB ${PROJECT_NAME})
  set(PRIVATE_LIBS "${PRIVATE_LIBS} -l${LIB}")
endforeach()
#CONFIGURE_FILE("${PROJECT_NAME}.pc.in" "${PROJECT_NAME}-${PROJECT_VERSION}.pc" @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.pc DESTINATION lib/pkgconfig)


