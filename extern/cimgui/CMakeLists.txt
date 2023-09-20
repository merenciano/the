cmake_minimum_required(VERSION 3.11)

project(cimgui)

set (CMAKE_CXX_STANDARD 11)

add_library(cimgui SHARED)

target_sources(cimgui PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/cimgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
)

target_compile_definitions(cimgui PUBLIC
	IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
	IMGUI_IMPL_API=extern\t\"C\"\t
)

target_include_directories(cimgui PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(cimgui PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(cimgui PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

