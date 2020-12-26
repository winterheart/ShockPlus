set(PALETTE_SRC
	Source/palette.c
	)

add_library(PALETTE STATIC ${PALETTE_SRC})
target_link_libraries(PALETTE 2D FIX LG)
target_include_directories(PALETTE PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/PALETTE/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS PALETTE FILE PALETTEConfig.cmake)
