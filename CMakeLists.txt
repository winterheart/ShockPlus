cmake_minimum_required(VERSION 3.11)

project(ShockPlus VERSION 0.8.0)

# Prefer new GLVND library interface
if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

# Required for stdbool.h
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
# For <filesystem> in C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIXME why this is required?
add_definitions(-D__STDC_LIMIT_MACROS)

option(ENABLE_DEBUG_BLIT "Enable debugging blitter" OFF)
option(ENABLE_OPENGL "Enable OpenGL support" ON)
option(ENABLE_SOUND "Enable sound support (requires SDL2_mixer)" ON)
option(ENABLE_LIBADLMIDI "Enable libADLMIDI support" OFF)
# Tristate Fluidsynth
set(ENABLE_FLUIDSYNTH "LITE" CACHE STRING "Enable FluidSynth MIDI support (ON/LITE/OFF, default LITE)")
set_property(CACHE ENABLE_FLUIDSYNTH PROPERTY STRINGS "ON" "LITE" "OFF")
option(ENABLE_WIN_CONSOLE "Enable debug console in Windows" ON)

# HAAAAX!!
add_definitions(-DSVGA_SUPPORT)

if(ENABLE_DEBUG_BLIT)
	add_definitions(-DDEBUGGING_BLIT)
endif()

add_compile_options(-fsigned-char -fno-strict-aliasing)

# Find OpenGL
if(ENABLE_OPENGL)
	find_package(OpenGL REQUIRED)
	add_definitions(-DUSE_OPENGL)
	list(APPEND SHOCK_INCLUDES ${OPENGL_INCLUDE_DIRS})
	list(APPEND SHOCK_LIBRARIES ${OPENGL_LIBRARIES})
	if(MINGW)
		set(GLEW_USE_STATIC_LIBS 1)
		find_package(GLEW REQUIRED)
		list(APPEND SHOCK_INCLUDES ${GLEW_INCLUDE_DIRS})
		list(APPEND SHOCK_LIBRARIES GLEW::GLEW)
	endif(MINGW)
endif(ENABLE_OPENGL)

find_package(SDL2 REQUIRED)
list(APPEND SHOCK_INCLUDES ${SDL2_INCLUDE_DIRS})
list(APPEND SHOCK_LIBRARIES ${SDL2_LIBRARIES})

if(ENABLE_SOUND)
	# FIXME applies only for *nix systems
	find_package(PkgConfig)
	pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer>=2.0.4)
	add_definitions(-DUSE_SDL_MIXER)
	list(APPEND SHOCK_INCLUDES ${SDL2_MIXER_INCLUDE_DIRS})
	list(APPEND SHOCK_LIBRARIES ${SDL2_MIXER_LIBRARIES})
endif(ENABLE_SOUND)

if(ENABLE_LIBADLMIDI)
	include(FetchContent)
	FetchContent_Declare(
		libadlmidi
		GIT_REPOSITORY https://github.com/Wohlstand/libADLMIDI.git
		GIT_SHALLOW ON
		GIT_TAG 723abef3094572c76c98cfb0f9e1eccc61af0d4c # 26.11.2020
	)
	FetchContent_GetProperties(libadlmidi)
	if(NOT libadlmidi_POPULATED)
		message(STATUS "Populating libADLMIDI...")
		FetchContent_Populate(libadlmidi)
		message(STATUS ${libadlmidi_SOURCE_DIR} ${libadlmidi_BINARY_DIR})
		add_subdirectory(${libadlmidi_SOURCE_DIR} ${libadlmidi_BINARY_DIR})
		message(STATUS "Done.")
	endif()
	include_directories(BEFORE SYSTEM ${libadlmidi_SOURCE_DIR}/include ${libadlmidi_BINARY_DIR}/include)
	add_definitions("-DUSE_LIBADLMIDI")
	list(APPEND SHOCK_LIBRARIES ADLMIDI_static)
endif(ENABLE_LIBADLMIDI)

if(ENABLE_FLUIDSYNTH STREQUAL "LITE")
	include(FetchContent)
	FetchContent_Declare(
		fluidsynth-lite
		# For time being https://github.com/Doom64/fluidsynth-lite/pull/10
		# https://github.com/Doom64/fluidsynth-lite
		GIT_REPOSITORY https://github.com/winterheart/fluidsynth-lite.git
		GIT_SHALLOW	ON
		GIT_TAG	a414e3055a652b5c8bb34b6e5b43e537468ccde6 # 26.01.2020
	)
	FetchContent_GetProperties(fluidsynth-lite)
	if(NOT fluidsynth-lite_POPULATED)
		message(STATUS "Populating fluidsynth-lite...")
		FetchContent_Populate(fluidsynth-lite)
		add_subdirectory(${fluidsynth-lite_SOURCE_DIR} ${fluidsynth-lite_BINARY_DIR})
		set_property(TARGET libfluidsynth PROPERTY C_STANDARD 11)
		message(STATUS "Done.")
	endif()
	include_directories(BEFORE SYSTEM ${fluidsynth-lite_SOURCE_DIR}/include ${fluidsynth-lite_BINARY_DIR}/include)
	add_definitions("-DUSE_FLUIDSYNTH")
	list(APPEND SHOCK_LIBRARIES libfluidsynth)
elseif(ENABLE_FLUIDSYNTH)
	# Use bulky system FluidSynth
	find_package(PkgConfig)
	pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth)
	list(APPEND SHOCK_INCLUDES ${FLUIDSYNTH_INCLUDE_DIRS})
	list(APPEND SHOCK_LIBRARIES ${FLUIDSYNTH_LIBRARIES})
	add_definitions("-DUSE_FLUIDSYNTH")
endif(ENABLE_FLUIDSYNTH STREQUAL "LITE")

if(NOT WIN32)
	# Find ALSA for Linux native MIDI
	# NOTE: this seems to require having 64-bit dev packages installed when building
	#  on 64-bit OS, even when building a 32-bit binary
	find_package(ALSA)
	if(ALSA_FOUND)
		message(STATUS "ALSA found")
		list(APPEND SHOCK_INCLUDES ${ALSA_INCLUDE_DIRS})
		list(APPEND SHOCK_LIBRARIES ${ALSA_LIBRARIES})
		add_definitions(-DUSE_ALSA)
	endif(ALSA_FOUND)
endif(NOT WIN32)

include_directories(
	${SHOCK_INCLUDES}
)

# Generate version based on project version
set(PROJECT_REVERSION_STRING "")
find_package(Git)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE git_describe_out
		ERROR_VARIABLE git_describe_error
		RESULT_VARIABLE git_describe_result
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	set(PROJECT_REVERSION_STRING "-g${git_describe_out}")
endif(GIT_FOUND)

message(STATUS "Version is ${PROJECT_VERSION}${PROJECT_REVERSION_STRING}")
configure_file("${CMAKE_SOURCE_DIR}/src/GameSrc/Headers/shock_version.h.in"
	"${CMAKE_BINARY_DIR}/src/GameSrc/Headers/shock_version.h" )

# Sources configuration
add_subdirectory(src/Libraries/)

set(MAC_SRC
	src/MacSrc/ShockBitmap.cpp
	src/MacSrc/InitMac.cpp
	src/MacSrc/Shock.cpp
	src/MacSrc/Prefs.cpp
	src/MacSrc/MacTune.cpp
	src/MacSrc/SDLSound.cpp
	src/MacSrc/Modding.cpp
	src/MacSrc/OpenGL.cpp
	src/MacSrc/Xmi.cpp
	src/MusicSrc/MusicDevice.cpp
)

set(GAME_SRC
	src/GameSrc/ai.cpp
	src/GameSrc/airupt.cpp
	src/GameSrc/amap.cpp
	src/GameSrc/amaploop.cpp
	src/GameSrc/ammomfd.cpp
	src/GameSrc/anim.cpp
	src/GameSrc/archiveformat.cpp
	src/GameSrc/audiolog.cpp
	src/GameSrc/automap.cpp
	src/GameSrc/bark.cpp
	src/GameSrc/biohelp.cpp
	src/GameSrc/cardmfd.cpp
	src/GameSrc/citres.cpp
	src/GameSrc/combat.cpp
	src/GameSrc/cone.cpp
	src/GameSrc/criterr.cpp
	src/GameSrc/cyber.cpp
	src/GameSrc/cybermfd.cpp
	src/GameSrc/cybmem.cpp
	src/GameSrc/cybrnd.cpp
	src/GameSrc/cutsloop.cpp
	src/GameSrc/damage.cpp
	src/GameSrc/digifx.cpp
	src/GameSrc/drugs.cpp
	src/GameSrc/effect.cpp
	src/GameSrc/email.cpp
	src/GameSrc/faceobj.cpp
	src/GameSrc/fixtrmfd.cpp
	src/GameSrc/frcamera.cpp
	src/GameSrc/frclip.cpp
	src/GameSrc/frcompil.cpp
	src/GameSrc/frmain.cpp
	src/GameSrc/frobj.cpp
	src/GameSrc/froslew.cpp
	src/GameSrc/frpipe.cpp
	src/GameSrc/frpts.cpp
	src/GameSrc/frsetup.cpp
	src/GameSrc/frtables.cpp
	src/GameSrc/frterr.cpp
	src/GameSrc/frutil.cpp
	src/GameSrc/FrUtils.cpp
	src/GameSrc/fullamap.cpp
	src/GameSrc/fullscrn.cpp
	src/GameSrc/gameloop.cpp
	src/GameSrc/gameobj.cpp
	src/GameSrc/gamesort.cpp
	src/GameSrc/gamestrn.cpp
	src/GameSrc/gamesys.cpp
	src/GameSrc/gametime.cpp
	src/GameSrc/gamewrap.cpp
	src/GameSrc/gearmfd.cpp
	src/GameSrc/gr2ss.cpp
	src/GameSrc/grenades.cpp
	src/GameSrc/hand.cpp
	src/GameSrc/hflip.cpp
	src/GameSrc/hkeyfunc.cpp
	src/GameSrc/hud.cpp
	src/GameSrc/hudobj.cpp
	src/GameSrc/init.cpp
	src/GameSrc/input.cpp
	src/GameSrc/invent.cpp
	src/GameSrc/leanmetr.cpp
	src/GameSrc/mainloop.cpp
	src/GameSrc/map.cpp
	src/GameSrc/mfdfunc.cpp
	src/GameSrc/mfdgadg.cpp
	src/GameSrc/mfdgames.cpp
	src/GameSrc/mfdgump.cpp
	src/GameSrc/mfdpanel.cpp
	src/GameSrc/minimax.cpp
	src/GameSrc/mlimbs.cpp
	src/GameSrc/movekeys.cpp
	src/GameSrc/musicai.cpp
	src/GameSrc/newai.cpp
	src/GameSrc/newmfd.cpp
	src/GameSrc/objapp.cpp
	src/GameSrc/objects.cpp
	src/GameSrc/objload.cpp
	src/GameSrc/objprop.cpp
	src/GameSrc/objsim.cpp
	src/GameSrc/objuse.cpp
	src/GameSrc/olh.cpp
	src/GameSrc/olhscan.cpp
	src/GameSrc/palfx.cpp
	src/GameSrc/pathfind.cpp
	src/GameSrc/physics.cpp
	src/GameSrc/player.cpp
	src/GameSrc/plotware.cpp
	src/GameSrc/popups.cpp
	src/GameSrc/render.cpp
	src/GameSrc/rendtool.cpp
	src/GameSrc/saveload.cpp
	src/GameSrc/schedule.cpp
	src/GameSrc/screen.cpp
	src/GameSrc/setup.cpp
	src/GameSrc/shodan.cpp
	src/GameSrc/sideicon.cpp
	src/GameSrc/sndcall.cpp
	src/GameSrc/star.cpp
	src/GameSrc/statics.cpp
	src/GameSrc/status.cpp
	src/GameSrc/target.cpp
	src/GameSrc/textmaps.cpp
	src/GameSrc/tickcount.cpp
	src/GameSrc/tfdirect.cpp
	src/GameSrc/tfutil.cpp
	src/GameSrc/tools.cpp
	src/GameSrc/trigger.cpp
	src/GameSrc/view360.cpp
	src/GameSrc/viewhelp.cpp
	src/GameSrc/vitals.cpp
	src/GameSrc/vmail.cpp
	src/GameSrc/wares.cpp
	src/GameSrc/weapons.cpp
	src/GameSrc/wrapper.cpp
	src/GameSrc/gamerend.cpp
	src/GameSrc/mouselook.cpp
)

include_directories(
	src/GameSrc/Headers
	src/Libraries/SND/Source
	src/MacSrc
	src/MusicSrc
	${CMAKE_BINARY_DIR}/src/GameSrc/Headers
)

# Include magic header file, set struct packing size
add_definitions(
	-include precompiled.h
)

add_executable(shockplus
	${GAME_SRC}
	${MAC_SRC}
)

# MINGW additional linker options
if(MINGW)
	set(WINDOWS_LIBRARIES mingw32 winmm)
	if(ENABLE_WIN_CONSOLE)
		list(APPEND ENABLE_WIN_CONSOLE -mwindows)
	endif(ENABLE_WIN_CONSOLE)
endif(MINGW)

target_link_libraries(shockplus
	${WINDOWS_LIBRARIES} # Set it before any linker options! Beware WinMain@16 error!!
	${SHOCK_LIBRARIES}
	2D
	3D
	AFILE
	DSTRUCT
	EDMS
	FIX
	FIXPP
	INPUT
	LG
	PALETTE
	RND
	RES
	UI
	VOX
)

# Turn on address sanitizing if wanted
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/sanitizers/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)
add_sanitizers(shockplus)
