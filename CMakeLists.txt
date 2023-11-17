cmake_minimum_required(VERSION 3.13)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

set(LIB_NAME nyas)

project(Nyas VERSION 0.1.0)

add_library(${LIB_NAME} STATIC)
set_target_properties(${LIB_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set_target_properties(${LIB_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_compile_definitions(${LIB_NAME} PUBLIC
	_GLFW_X11
	LUA_USE_POSIX
    NYAS_DEBUG
	)

target_compile_options(${LIB_NAME} PUBLIC
		-Wall
	)

target_include_directories(${LIB_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/extern/include
	)

add_subdirectory(src)
add_subdirectory(extern)
add_subdirectory(examples)
