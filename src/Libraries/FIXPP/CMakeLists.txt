set(FIXPP_SRC
	Source/fixpp.cpp
	)
add_library(FIXPP STATIC ${FIXPP_SRC})
target_link_libraries(FIXPP FIX)
target_include_directories(FIXPP PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/FIXPP/Source
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS FIXPP FILE FIXPPConfig.cmake)
