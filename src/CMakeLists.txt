project(stalk-api)

cmake_minimum_required(VERSION 3.4)

find_package(Threads REQUIRED)

find_package(PkgConfig)

pkg_check_modules(OPENSSL openssl)
include_directories(${OPENSSL_INCLUDE_DIRS})
link_directories(${OPENSSL_LIBRARY_DIRS})

pkg_check_modules(STALK stalk-1.0)
include_directories(${STALK_INCLUDE_DIRS})
link_directories(${STALK_LIBRARY_DIRS})

include_directories(${LURLPARSER_INCLUDEDIR})
include_directories(${SPDLOG_INCLUDEDIR})
include_directories(${JSON_INCLUDEDIR})
include_directories(${LOGGER_INCLUDEDIR})
include_directories(${CXXOPTS_INCLUDEDIR})
include_directories(${PROMETHEUS_INCLUDEDIR})

set(SOURCE_FILES
    main.cpp
    core_routes.cpp
    metrics_instance.cpp
    middleware/middleware.cpp
    middleware/middleware_delayer.cpp
    middleware/middleware_metrics.cpp
    middleware/middleware_secure_header_hoister.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    LUrlParser
    logger
    prometheus-cpp
    ${Boost_LIBRARIES}
    ${STALK_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_dependencies(${PROJECT_NAME} LUrlParser logger)
