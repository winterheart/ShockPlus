set(RES_SRC
	Source/caseless.c
	Source/lzw.c
	Source/refacc.c
	Source/resacc.c
	Source/resbuild.c
	Source/res.c
	Source/resfile.c
	Source/resformat.c
	Source/resload.c
	Source/resmake.c
	Source/restypes.c
	)

add_library(RES ${RES_SRC})
target_link_libraries(RES 2D LG)
target_include_directories(RES PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/RES/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS RES FILE RESConfig.cmake)