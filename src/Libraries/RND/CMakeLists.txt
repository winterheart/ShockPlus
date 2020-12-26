set(RND_SRC
	Source/rnd.c
	)
add_library(RND STATIC ${RND_SRC})
target_link_libraries(RND FIX LG)
target_include_directories(RND PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/RND/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS RND FILE RNDConfig.cmake)
