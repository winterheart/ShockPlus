set(3D_SRC
	Source/alloc.c
	Source/Bitmap.c
	Source/clip.c
	Source/detail.c
	Source/fov.c
	Source/GlobalV.c
	Source/instance.c
	Source/interp.c
	Source/light.c
	Source/matrix.c
	Source/points.c
	Source/polygon.c
	Source/slew.c
	Source/tmap.c
	Source/vector.c
	)

add_library(3D ${3D_SRC})
target_link_libraries(3D 2D FIX LG m)
target_include_directories(3D PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/3D/Source
	${CMAKE_SOURCE_DIR}/src/MacSrc
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS 3D FILE 3DConfig.cmake)
