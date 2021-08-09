set(UI_SRC
	Source/curdrw.c
	Source/cursors.c
	Source/event_ui.c
	Source/region.c
	Source/slab.c
	Source/vmouse.c
	)

add_library(UI STATIC ${UI_SRC})
target_link_libraries(UI DSTRUCT INPUT LG)
target_include_directories(UI PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/H
	${CMAKE_SOURCE_DIR}/src/Libraries/UI/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS UI FILE UIConfig.cmake)
