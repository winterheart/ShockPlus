set(EDMS_SRC
	Source/interfac.cc
	Source/collide.cc
	Source/intrsect.cc
	Source/globals.cc
	Source/phy_tool.cc
	Source/soliton.cc
	Source/MODELS/robot.cc
	Source/MODELS/pelface.cc
	Source/MODELS/ftl.cc
	Source/MODELS/pelvis.cc
	Source/MODELS/d_frame.cc
	Source/MODELS/d_f_face.cc
	Source/MODELS/ftlface.cc
	Source/MODELS/d_f_2.cc
	)
add_library(EDMS STATIC ${EDMS_SRC})
target_link_libraries(EDMS FIXPP)
target_include_directories(EDMS PUBLIC
	$<BUILD_INTERFACE:
	${CMAKE_SOURCE_DIR}/src/Libraries/EDMS/Source
	${CMAKE_SOURCE_DIR}/src/Libraries/EDMS/Source/MODELS
	>
	$<INSTALL_INTERFACE:include>
	)
export(TARGETS EDMS FILE EDMSConfig.cmake)