set(DSTRUCT_SRC
	Source/array.c
	Source/hash.c
	Source/pqueue.c
	Source/rect.c
	)

add_library(DSTRUCT STATIC ${DSTRUCT_SRC})
target_link_libraries(DSTRUCT LG)
target_include_directories(DSTRUCT PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/DSTRUCT/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS DSTRUCT FILE DSTRUCTConfig.cmake)


