cmake_minimum_required(VERSION 3.13)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(NyasCPP VERSION 0.1.0)

add_executable(nyas_cpp main.cpp)
set_target_properties(nyas_cpp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(nyas_cpp PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(nyas_cpp PUBLIC
        -Wall
        -Wextra
        -Wpedantic
)

target_include_directories(nyas_cpp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_directories(nyas_cpp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
)

target_link_libraries(nyas_cpp PUBLIC
        nyas
        X11
        dl
        pthread
)