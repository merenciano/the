set(TEST test)
project(${TEST} VERSION 1.0.0)
add_executable(${TEST} ${CMAKE_CURRENT_SOURCE_DIR}/test.c)

set_target_properties(${TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${TEST} PUBLIC -Wall -Wpedantic)
target_include_directories(${TEST} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
	)

target_link_libraries(${TEST} PUBLIC
	nyas
	GL
	X11
	dl
	pthread
	m
	)

