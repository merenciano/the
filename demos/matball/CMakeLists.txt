set(MATBALLS matballs)
project(${MATBALLS} VERSION 0.0.1)
add_executable(${MATBALLS} ${CMAKE_CURRENT_SOURCE_DIR}/matballs.c)

set_target_properties(${MATBALLS} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${MATBALLS} PUBLIC -Wall -Wpedantic)
target_include_directories(${MATBALLS} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
	)

target_link_libraries(${MATBALLS} PUBLIC
	nyas
	GL
	X11
	dl
	pthread
	m
	)

