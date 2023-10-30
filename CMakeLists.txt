cmake_minimum_required(VERSION 3.13)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

project(The VERSION 0.1.0)

set(THE_NAME the)
add_library(${THE_NAME} STATIC)
set_target_properties(${THE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set_target_properties(${THE_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_compile_definitions(${THE_NAME} PUBLIC
	_GLFW_X11
	LUA_USE_POSIX
	)

target_compile_options(${THE_NAME} PUBLIC
		-Wall
	)

target_include_directories(${THE_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/extern/include
	)

add_subdirectory(src)
add_subdirectory(extern)
add_subdirectory(examples)
