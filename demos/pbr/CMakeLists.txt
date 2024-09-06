project(pbr_demo VERSION 1.0.0)
add_executable(pbr ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
set_target_properties(pbr PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(pbr PRIVATE -Wall -Wpedantic)
target_include_directories(pbr PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
)
target_link_libraries(pbr PRIVATE
	nyas
	GL
	X11
	dl
	pthread
	m
)
target_sources(pbr PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
	${CMAKE_CURRENT_SOURCE_DIR}/gui.c
)

