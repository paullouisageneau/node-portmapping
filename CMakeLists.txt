cmake_minimum_required(VERSION 3.13)
project(node_portmapping VERSION 0.1.1)

include_directories(${CMAKE_JS_INC})

set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

FetchContent_Declare(
    libplum
    GIT_REPOSITORY https://github.com/paullouisageneau/libplum.git
    GIT_TAG "v0.1.3"
)

FetchContent_GetProperties(libplum)
if(NOT libplum)
    FetchContent_Populate(libplum)
    add_subdirectory(${libplum_SOURCE_DIR} ${libplum_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_library(node_portmapping SHARED
    src/wrapper.cpp
    src/mapping.cpp
    src/callback.cpp
    src/main.cpp
    ${CMAKE_JS_SRC}
)

set_target_properties(node_portmapping PROPERTIES CXX_STANDARD 17)
set_target_properties(node_portmapping PROPERTIES PREFIX "" SUFFIX ".node")

target_include_directories(node_portmapping PRIVATE
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
    ${CMAKE_BINARY_DIR}/_deps/libplum-src/include
)

target_link_libraries(node_portmapping PRIVATE
    plum-static
    ${CMAKE_JS_LIB}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(node_portmapping PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()

