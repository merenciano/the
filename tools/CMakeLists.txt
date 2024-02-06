project (nyas_tools VERSION 1.0.0)
add_executable(genenv)
set_target_properties(genenv PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(genenv PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../extern/include
)

target_sources(genenv PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/genenv.c
	${CMAKE_CURRENT_SOURCE_DIR}/../extern/src/mathc.c
	${CMAKE_CURRENT_SOURCE_DIR}/../extern/src/glad.c
)

target_link_libraries(genenv PRIVATE
	nyas_d
	GL
	X11
	m
)

add_executable(tomesh)
set_target_properties(tomesh PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(tomesh PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../extern/include
)

target_sources(tomesh PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/tomesh.c
)

target_link_libraries(genenv PRIVATE
	m
)
