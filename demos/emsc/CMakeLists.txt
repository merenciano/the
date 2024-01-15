project(emsc VERSION 1.0.0)
set(CMAKE_EXECUTABLE_SUFFIX ".html")
add_executable(emsc ${CMAKE_CURRENT_SOURCE_DIR}/emsc_demo.c)

set_target_properties(emsc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(emsc PRIVATE -Wall)
target_include_directories(emsc PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../extern/include
	)

target_link_libraries(emsc PRIVATE
	nyas
	dl
	pthread
	m
)

target_link_options(emsc PRIVATE
	--no-heap-copy
	--preload-file ../../../../demos/emsc/assets@/assets
	-sUSE_GLFW=3
	-sALLOW_MEMORY_GROWTH=1
	-sWASM=1
	-sASSERTIONS=1
)
