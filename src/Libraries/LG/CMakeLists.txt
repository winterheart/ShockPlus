set(LG_SRC
	Source/LOG/src/log.c
	Source/stack.c
	Source/tmpalloc.c
	)

add_library(LG STATIC ${LG_SRC})
target_include_directories(LG PUBLIC
	$<BUILD_INTERFACE:
		${CMAKE_SOURCE_DIR}/src/Libraries/LG/Source
		${CMAKE_SOURCE_DIR}/src/Libraries/LG/Source/LOG/src
		${CMAKE_SOURCE_DIR}/src/Libraries/H
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS LG FILE LGConfig.cmake)
