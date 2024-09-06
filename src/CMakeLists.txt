target_sources(${LIB_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/nyas.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/io.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/io.c
	${CMAKE_CURRENT_SOURCE_DIR}/core/mem.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/mem.c
	${CMAKE_CURRENT_SOURCE_DIR}/core/scene.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/scene.c
	${CMAKE_CURRENT_SOURCE_DIR}/core/sched.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/sched.c
	${CMAKE_CURRENT_SOURCE_DIR}/core/utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/utils.c
	${CMAKE_CURRENT_SOURCE_DIR}/render/pixels.h
	${CMAKE_CURRENT_SOURCE_DIR}/render/pixels.c
	${CMAKE_CURRENT_SOURCE_DIR}/render/pixels_internal.h
)
