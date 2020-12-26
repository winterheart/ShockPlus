set(AFILE_SRC
	Source/afile.c
	Source/amov.c
	Source/compose.c
	Source/draw4x4.cpp
	Source/huffde.cpp
	Source/movie.c
	)

add_library(AFILE STATIC ${AFILE_SRC})
target_link_libraries(AFILE 2D DSTRUCT RES)
target_include_directories(AFILE PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/AFILE/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS AFILE FILE AFILEConfig.cmake)