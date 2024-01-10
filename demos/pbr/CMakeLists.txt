project(pbr_demos VERSION 1.0.0)
add_executable(matballs ${CMAKE_CURRENT_SOURCE_DIR}/matballs.c)
set_target_properties(matballs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(matballs PRIVATE -Wall -Wpedantic)
target_include_directories(matballs PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
)
target_link_libraries(matballs PRIVATE
	nyas
	GL
	X11
	dl
	pthread
	m
)
target_sources(matballs PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/matballs.c
	${CMAKE_CURRENT_SOURCE_DIR}/gui.c
)

add_executable(inspector ${CMAKE_CURRENT_SOURCE_DIR}/inspector.c)
set_target_properties(inspector PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(inspector PRIVATE -Wall -Wpedantic)
target_include_directories(inspector PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
)
target_link_libraries(inspector PRIVATE
	nyas
	GL
	X11
	dl
	pthread
	m
)
target_sources(inspector PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/inspector.c
	${CMAKE_CURRENT_SOURCE_DIR}/gui.c
)
