set(HELLO hello)
project(${HELLO} VERSION 0.0.1)
add_executable(${HELLO} ${CMAKE_CURRENT_SOURCE_DIR}/hello.c)

set_target_properties(${HELLO} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${HELLO} PUBLIC -Wall -Wpedantic)
target_include_directories(${HELLO} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
	)

target_link_libraries(${HELLO} PUBLIC
	nyas
	GL
	X11
	dl
	pthread
	m
	)
