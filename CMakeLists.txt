cmake_minimum_required(VERSION 3.13)
project(node_portmapping VERSION 0.2.1)

include_directories(${CMAKE_JS_INC})

set(CMAKE_BUILD_TYPE Release)

include(FetchContent)

FetchContent_Declare(
    libplum
    GIT_REPOSITORY https://github.com/paullouisageneau/libplum.git
    GIT_TAG "v0.2.3"
)

FetchContent_GetProperties(libplum)
if(NOT libplum_POPULATED)
    FetchContent_Populate(libplum)
    add_subdirectory(${libplum_SOURCE_DIR} ${libplum_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_library(${PROJECT_NAME} SHARED
    src/wrapper.cpp
    src/mapping.cpp
    src/callback.cpp
    src/main.cpp
    ${CMAKE_JS_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_link_libraries(${PROJECT_NAME} PRIVATE
    LibPlum::LibPlumStatic
    ${CMAKE_JS_LIB}
)

# Include node-addon-api wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()

