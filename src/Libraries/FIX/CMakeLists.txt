set(FIX_SRC
	Source/f_exp.c
	Source/fix.c
	Source/fix_pow.c
	Source/fix_sqrt.c
	Source/MakeTables.c
	)

add_library(FIX STATIC ${FIX_SRC})
target_link_libraries(FIX LG m)
target_include_directories(FIX PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/FIX/Source
#	${CMAKE_SOURCE_DIR}/src/Libraries/H
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS FIX FILE FIXConfig.cmake)
