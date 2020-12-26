set(VOX_SRC
	Source/vox2d.c
	Source/vox3d.c
	Source/voxinit.c
	)
add_library(VOX STATIC ${VOX_SRC})
target_link_libraries(VOX 2D 3D LG)
target_include_directories(VOX PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/VOX/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS VOX FILE VOXConfig.cmake)
