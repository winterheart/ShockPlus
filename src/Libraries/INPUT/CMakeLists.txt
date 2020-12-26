set(INPUT_SRC
	Source/kbcook.c
	Source/mouse.c
	Source/sdl_events.c
	)
add_library(INPUT STATIC ${INPUT_SRC})
target_link_libraries(INPUT 3D LG)
target_include_directories(INPUT PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/GameSrc/Headers
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/INPUT/Source
	${CMAKE_SOURCE_DIR}/src/MacSrc
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS INPUT FILE INPUTConfig.cmake)
