###############################################################################
# Hero of Allacrost - CMake Build File
#
# This file contains all of the configurations needed to build both the game and
# editor using CMake. Refer to the README file for instructions on how to use.
#
# Build types:
#   "develop" - use for developers. Enables debugging features and compiles more quickly.
#   "release" - use for official releases. Optimizes produced binaries for performance.
#
# Build options:
#   "EDITOR" - build the map editor in addition to the game (default = ON)
#   "USEPCH" - build using pre-compiled header files (default = ON)
###############################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
CMAKE_POLICY(VERSION 3.5)

PROJECT(ALLACROST)

###############################################################################
# Build Options -- Release Versions
###############################################################################
##### Set the build type to "develop" if unspecified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "develop")
endif()
message(STATUS "Allacrost build type: ${CMAKE_BUILD_TYPE}")

##### Build options that can be set
option(EDITOR "Build the map editor in addition to the game" ON)
option(USEPCH "Using precompiled header for compilation for GCC" ON)

##### Set the release version number for the project. Change this before every official release.
set(VERSION 1.0.0)
string(REPLACE "." " " _VERSION ${VERSION})
separate_arguments(_VERSION)
list(LENGTH _VERSION _LEN)
if(NOT _LEN EQUAL 3)
	message(FATAL_ERROR "Version needs to be in the form MAJOR.MINOR.RELEASE")
endif()

list(GET _VERSION 0 VER_MAJOR)
list(GET _VERSION 1 VER_MINOR)
list(GET _VERSION 2 VER_RELEASE)

##### Set the paths where the game data, binaries, and locales will be located
set(PKG_NAME "allacrost")
if(WIN32)
	set(PKG_DATADIR ".")
	set(PKG_BINDIR ".")
	set(LOCALEDIR ".")
	configure_file(src/winver.h.in src/winver.h)
else()
	if(NOT OSX AND NOT BEOS)
		option(USE_X11 "Use X11 Clipboard functionality" OFF)
	endif()
	set(PKG_DATADIR ${CMAKE_INSTALL_PREFIX}/share/allacrost CACHE PATH "Data directory")
	set(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale CACHE PATH "Locale directory")
	set(PKG_BINDIR ${CMAKE_INSTALL_PREFIX}/bin CACHE PATH "Binary directory")
endif()

##### Handle the case where a user overwrites these paths and specifies a relative path instead
foreach(dir PKG_DATADIR LOCALEDIR)
	if(WIN32)
		# No change for Windows
		set(FULL_${dir} ${${dir}})
	else()
		if(NOT IS_ABSOLUTE ${${dir}})
			set(FULL_${dir} "${CMAKE_INSTALL_PREFIX}/${${dir}}")
		else()
			set(FULL_${dir} ${${dir}})
		endif()
	endif()
endforeach()


###############################################################################
# Library Dependencies
###############################################################################
##### For Debian Linux systems, automatically install any required development libraries with the apt package manager
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_file (DEBIAN_VERSION NAMES debian_version PATHS /etc NO_DEFAULT_PATH)
	if(${DEBIAN_VERSION} STREQUAL "/etc/debian_version")
		message(STATUS "Debian Linux system detected, checking installed packages")
		# Install dev target to ease development setup
		add_custom_target(install_debian_packages
			sudo apt install libsdl2-ttf-dev libsdl2-image-dev libgl1-mesa-dev libopenal-dev libvorbis-dev liblua5.1-dev libpng-dev gettext libboost-dev
			COMMENT "Installing Debian development library dependencies ..."
			VERBATIM
		)
	endif()
endif()


##### Path to where the custom Cmake modules are located
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build/cmake/modules)
message(STATUS "Searching for library dependencies...")

find_package(Boost 1.46.1 REQUIRED)
find_package(Gettext REQUIRED)
find_package(JPEG REQUIRED)
find_package(Lua51 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(VorbisFile REQUIRED)

# The FindBoost module already prints the boost version found
message(STATUS "gettext version ${GETTEXT_VERSION_STRING} - found at ${GETTEXT_MSGFMT_EXECUTABLE}")
message(STATUS "libjpeg - found at ${JPEG_INCLUDE_DIR}")
message(STATUS "liblua version ${LUA_VERSION_STRING} - found at ${LUA_INCLUDE_DIR}")
message(STATUS "libpng version ${PNG_VERSION_STRING} - found at ${PNG_INCLUDE_DIRS}")
message(STATUS "libsdl version ${SDL2_VERSION_STRING} - found at ${SDL2_INCLUDE_DIRS}")
message(STATUS "libsdl_ttf version ${SDL2_TTF_VERSION_STRING} - found at ${SDL2_TTF_INCLUDE_DIRS}")
message(STATUS "libvorbisfile - found at ${VORBISFILE_INCLUDE_DIR}")
message(STATUS "opengl - found at ${OPENGL_INCLUDE_DIR}")
message(STATUS "openal - found at ${OPENAL_INCLUDE_DIR}")

# OpenBSD requires Ogg and Vorbis packages in addition to VorbisFile
if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	find_package(Ogg REQUIRED)
	find_package(Vorbis REQUIRED)

	message(STATUS "libogg - found at ${OGG_INCLUDE_DIR}")
	message(STATUS "libvorbis - found at ${VORBIS_INCLUDE_DIR}")
endif()

# X11 Window System
if(USE_X11)
	find_package(X11 REQUIRED)
	include_directories(${X11_INCLUDE_DIR})
	set(FLAGS "${FLAGS} -DUSE_X11")

	message(STATUS "x11 - found at ${X11_INCLUDE_DIR}")
endif()

# Internationalization libraries for various systems
if(WIN32)
	set(EXTRA_LIBRARIES ws2_32 winmm)
	find_package(LibIntl REQUIRED)

	message(STATUS "libintl - found at ${LIBINTL_LIBRARIES}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR BEOS OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	find_package(LibIntl REQUIRED)
	find_package(LibIconv REQUIRED)

	message(STATUS "libintl - found at ${LIBINTL_LIBRARIES}")
	message(STATUS "libiconv - found at ${ICONV_INCLUDE_DIR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	find_package(LibIntl REQUIRED)

	message(STATUS "libintl - found at ${LIBINTL_LIBRARIES}")
elseif(CMAKE_SYSTEM_NAME STREQUAL SunOS)
	set(EXTRA_LIBRARIES intl)
endif()

# Look for QT only if the editor is being built
if(EDITOR)
	find_package(Qt5Core)
	find_package(Qt5Gui)
	find_package(Qt5OpenGL)
	message(STATUS "libqt5 found at ${Qt5Core_INCLUDE_DIRS}")
endif()

# Fix some packages that do get not hidden by mainline modules
mark_as_advanced(SDLMAIN_LIBRARY)
mark_as_advanced(SDL_INCLUDE_DIR)
mark_as_advanced(SDL_LIBRARY)

message(STATUS "All library dependecies were found")


###############################################################################
# Allacrost Header Directories -- Allacrost Source Files
###############################################################################
# Include file paths for libraries and the Allacrost source
set(ALLACROST_HEADER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/src/
	${CMAKE_CURRENT_SOURCE_DIR}/src/common
	${CMAKE_CURRENT_SOURCE_DIR}/src/common/global
	${CMAKE_CURRENT_SOURCE_DIR}/src/common/gui
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/script
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/audio
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/video
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind/luabind
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind/luabind/detail
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes/battle
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes/map
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes/shop
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes/boot
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes/save
	${CMAKE_CURRENT_SOURCE_DIR}/src/modes/menu
)

set(EDITOR_HEADER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/src/
	${CMAKE_CURRENT_SOURCE_DIR}/src/engine/script
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind/luabind
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind/luabind/detail
	${CMAKE_CURRENT_SOURCE_DIR}/src/luabind/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/editor
)

# Luabind Library (built directly into the binaries)
set(SOURCES_LUABIND
	src/luabind/luabind/adopt_policy.hpp
	src/luabind/luabind/back_reference_fwd.hpp
	src/luabind/luabind/back_reference.hpp
	src/luabind/luabind/class.hpp
	src/luabind/luabind/class_info.hpp
	src/luabind/luabind/config.hpp
	src/luabind/luabind/container_policy.hpp
	src/luabind/luabind/copy_policy.hpp
	src/luabind/luabind/dependency_policy.hpp
	src/luabind/luabind/discard_result_policy.hpp
	src/luabind/luabind/error_callback_fun.hpp
	src/luabind/luabind/error.hpp
	src/luabind/luabind/exception_handler.hpp
	src/luabind/luabind/from_stack.hpp
	src/luabind/luabind/function.hpp
	src/luabind/luabind/function_introspection.hpp
	src/luabind/luabind/get_main_thread.hpp
	src/luabind/luabind/get_pointer.hpp
	src/luabind/luabind/handle.hpp
	src/luabind/luabind/iterator_policy.hpp
	src/luabind/luabind/luabind.hpp
	src/luabind/luabind/lua_include.hpp
	src/luabind/luabind/lua_state_fwd.hpp
	src/luabind/luabind/make_function.hpp
	src/luabind/luabind/nil.hpp
	src/luabind/luabind/no_dependency.hpp
	src/luabind/luabind/object.hpp
	src/luabind/luabind/open.hpp
	src/luabind/luabind/operator.hpp
	src/luabind/luabind/out_value_policy.hpp
	src/luabind/luabind/prefix.hpp
	src/luabind/luabind/raw_policy.hpp
	src/luabind/luabind/return_reference_to_policy.hpp
	src/luabind/luabind/scope.hpp
	src/luabind/luabind/set_package_preload.hpp
	src/luabind/luabind/shared_ptr_converter.hpp
	src/luabind/luabind/std_shared_ptr_converter.hpp
	src/luabind/luabind/tag_function.hpp
	src/luabind/luabind/typeid.hpp
	src/luabind/luabind/value_wrapper.hpp
	src/luabind/luabind/version.hpp
	src/luabind/luabind/weak_ref.hpp
	src/luabind/luabind/wrapper_base.hpp
	src/luabind/luabind/yield_policy.hpp

	src/luabind/luabind/detail/call_function.hpp
	src/luabind/luabind/detail/call.hpp
	src/luabind/luabind/detail/call_member.hpp
	src/luabind/luabind/detail/call_operator_iterate.hpp
	src/luabind/luabind/detail/class_registry.hpp
	src/luabind/luabind/detail/class_rep.hpp
	src/luabind/luabind/detail/constructor.hpp
	src/luabind/luabind/detail/conversion_storage.hpp
	src/luabind/luabind/detail/convert_to_lua.hpp
	src/luabind/luabind/detail/debug.hpp
	src/luabind/luabind/detail/decorate_type.hpp
	src/luabind/luabind/detail/deduce_signature.hpp
	src/luabind/luabind/detail/enum_maker.hpp
	src/luabind/luabind/detail/format_signature.hpp
	src/luabind/luabind/detail/garbage_collector.hpp
	src/luabind/luabind/detail/has_get_pointer.hpp
	src/luabind/luabind/detail/inheritance.hpp
	src/luabind/luabind/detail/instance_holder.hpp
	src/luabind/luabind/detail/link_compatibility.hpp
	src/luabind/luabind/detail/make_instance.hpp
	src/luabind/luabind/detail/most_derived.hpp
	src/luabind/luabind/detail/object_call.hpp
	src/luabind/luabind/detail/object.hpp
	src/luabind/luabind/detail/object_rep.hpp
	src/luabind/luabind/detail/open.hpp
	src/luabind/luabind/detail/operator_id.hpp
	src/luabind/luabind/detail/other.hpp
	src/luabind/luabind/detail/pcall.hpp
	src/luabind/luabind/detail/pointee_sizeof.hpp
	src/luabind/luabind/detail/pointee_typeid.hpp
	src/luabind/luabind/detail/policy.hpp
	src/luabind/luabind/detail/primitives.hpp
	src/luabind/luabind/detail/property.hpp
	src/luabind/luabind/detail/ref.hpp
	src/luabind/luabind/detail/signature_match.hpp
	src/luabind/luabind/detail/stack_utils.hpp
	src/luabind/luabind/detail/typetraits.hpp
	src/luabind/luabind/detail/yes_no.hpp

	src/luabind/src/class.cpp
	src/luabind/src/class_info.cpp
	src/luabind/src/class_registry.cpp
	src/luabind/src/class_rep.cpp
	src/luabind/src/create_class.cpp
	src/luabind/src/error.cpp
	src/luabind/src/exception_handler.cpp
	src/luabind/src/function.cpp
	src/luabind/src/function_introspection.cpp
	src/luabind/src/inheritance.cpp
	src/luabind/src/link_compatibility.cpp
	src/luabind/src/object_rep.cpp
	src/luabind/src/open.cpp
	src/luabind/src/operator.cpp
	src/luabind/src/pcall.cpp
	src/luabind/src/scope.cpp
	src/luabind/src/set_package_preload.cpp
	src/luabind/src/stack_content_by_name.cpp
	src/luabind/src/weak_ref.cpp
	src/luabind/src/wrapper_base.cpp
)

##### Allacrost Engines
set(SOURCES_AUDIO_ENGINE
	src/engine/audio/audio.cpp
	src/engine/audio/audio.h
	src/engine/audio/audio_descriptor.cpp
	src/engine/audio/audio_descriptor.h
	src/engine/audio/audio_effects.cpp
	src/engine/audio/audio_effects.h
	src/engine/audio/audio_input.cpp
	src/engine/audio/audio_input.h
	src/engine/audio/audio_stream.cpp
	src/engine/audio/audio_stream.h
)

set(SOURCES_SCRIPT_ENGINE
	src/engine/script/script.cpp
	src/engine/script/script.h
	src/engine/script/script_read.cpp
	src/engine/script/script_read.h
	src/engine/script/script_write.cpp
	src/engine/script/script_write.h
	src/engine/script/script_modify.cpp
	src/engine/script/script_modify.h
)

set(SOURCES_VIDEO_ENGINE
	src/engine/video/color.h
	src/engine/video/context.h
	src/engine/video/coord_sys.h
	src/engine/video/effects.cpp
	src/engine/video/fade.cpp
	src/engine/video/fade.h
	src/engine/video/image_base.cpp
	src/engine/video/image_base.h
	src/engine/video/image.cpp
	src/engine/video/image.h
	src/engine/video/interpolator.cpp
	src/engine/video/interpolator.h
	src/engine/video/particle.h
	src/engine/video/particle_effect.cpp
	src/engine/video/particle_effect.h
	src/engine/video/particle_emitter.h
	src/engine/video/particle_keyframe.h
	src/engine/video/particle_manager.cpp
	src/engine/video/particle_manager.h
	src/engine/video/particle_system.cpp
	src/engine/video/particle_system.h
	src/engine/video/screen_rect.h
	src/engine/video/shake.cpp
	src/engine/video/shake.h
	src/engine/video/text.cpp
	src/engine/video/text.h
	src/engine/video/texture.cpp
	src/engine/video/texture.h
	src/engine/video/texture_controller.cpp
	src/engine/video/texture_controller.h
	src/engine/video/video.cpp
	src/engine/video/video.h
)

set(SOURCES_ENGINE
	${SOURCES_AUDIO_ENGINE}
	${SOURCES_SCRIPT_ENGINE}
	${SOURCES_VIDEO_ENGINE}
	src/engine/engine_bindings.cpp
	src/engine/input.cpp
	src/engine/input.h
	src/engine/mode_manager.cpp
	src/engine/mode_manager.h
	src/engine/notification.cpp
	src/engine/notification.h
	src/engine/system.cpp
	src/engine/system.h
)

##### Allacrost Game Modes
set(SOURCES_BATTLE_MODE
	src/modes/battle/battle.cpp
	src/modes/battle/battle.h
	src/modes/battle/battle_actions.cpp
	src/modes/battle/battle_actions.h
	src/modes/battle/battle_actors.cpp
	src/modes/battle/battle_actors.h
	src/modes/battle/battle_command.cpp
	src/modes/battle/battle_command.h
	src/modes/battle/battle_dialogue.cpp
	src/modes/battle/battle_dialogue.h
	src/modes/battle/battle_effects.cpp
	src/modes/battle/battle_effects.h
	src/modes/battle/battle_finish.cpp
	src/modes/battle/battle_finish.h
	src/modes/battle/battle_indicators.cpp
	src/modes/battle/battle_indicators.h
	src/modes/battle/battle_sequence.cpp
	src/modes/battle/battle_sequence.h
	src/modes/battle/battle_utils.cpp
	src/modes/battle/battle_utils.h
)

set(SOURCES_BOOT_MODE
	src/modes/boot/boot.cpp
	src/modes/boot/boot.h
	src/modes/boot/boot_menu.cpp
	src/modes/boot/boot_menu.h
	src/modes/boot/boot_credits.cpp
	src/modes/boot/boot_credits.h
)

set(SOURCES_MAP_MODE
	src/modes/map/map.cpp
	src/modes/map/map.h
	src/modes/map/map_dialogue.cpp
	src/modes/map/map_dialogue.h
	src/modes/map/map_events.cpp
	src/modes/map/map_events.h
	src/modes/map/map_objects.cpp
	src/modes/map/map_objects.h
	src/modes/map/map_sprites.cpp
	src/modes/map/map_sprites.h
	src/modes/map/map_sprite_events.cpp
	src/modes/map/map_sprite_events.h
	src/modes/map/map_tiles.cpp
	src/modes/map/map_tiles.h
	src/modes/map/map_transition.cpp
	src/modes/map/map_transition.h
	src/modes/map/map_treasure.cpp
	src/modes/map/map_treasure.h
	src/modes/map/map_utils.cpp
	src/modes/map/map_utils.h
	src/modes/map/map_zones.cpp
	src/modes/map/map_zones.h
)

set(SOURCES_MENU_MODE
	src/modes/menu/menu.cpp
	src/modes/menu/menu.h
	src/modes/menu/menu_formation.cpp
	src/modes/menu/menu_formation.h
	src/modes/menu/menu_character.cpp
	src/modes/menu/menu_character.h
	src/modes/menu/menu_equip.cpp
	src/modes/menu/menu_equip.h
	src/modes/menu/menu_inventory.cpp
	src/modes/menu/menu_inventory.h
	src/modes/menu/menu_skills.cpp
	src/modes/menu/menu_skills.h
	src/modes/menu/menu_status.cpp
	src/modes/menu/menu_status.h
)

set(SOURCES_SAVE_MODE
	src/modes/save/save_mode.h
	src/modes/save/save_mode.cpp
)

set(SOURCES_SHOP_MODE
	src/modes/shop/shop.h
	src/modes/shop/shop.cpp
	src/modes/shop/shop_buy.h
	src/modes/shop/shop_buy.cpp
	src/modes/shop/shop_confirm.h
	src/modes/shop/shop_confirm.cpp
	src/modes/shop/shop_leave.h
	src/modes/shop/shop_leave.cpp
	src/modes/shop/shop_root.h
	src/modes/shop/shop_root.cpp
	src/modes/shop/shop_sell.h
	src/modes/shop/shop_sell.cpp
	src/modes/shop/shop_trade.h
	src/modes/shop/shop_trade.cpp
	src/modes/shop/shop_utils.h
	src/modes/shop/shop_utils.cpp
)

set(SOURCES_MODES
	${SOURCES_BATTLE_MODE}
	${SOURCES_BOOT_MODE}
	${SOURCES_MAP_MODE}
	${SOURCES_MENU_MODE}
	${SOURCES_SAVE_MODE}
	${SOURCES_SHOP_MODE}
	src/modes/custom.cpp
	src/modes/custom.h
	src/modes/mode_bindings.cpp
	src/modes/pause.cpp
	src/modes/pause.h
	src/modes/scene.cpp
	src/modes/scene.h
	src/modes/test.cpp
	src/modes/test.h
)

##### Allacrost Common Code
set(SOURCES_GLOBAL_COMMON
	src/common/global/global.cpp
	src/common/global/global.h
	src/common/global/global_actors.cpp
	src/common/global/global_actors.h
	src/common/global/global_effects.cpp
	src/common/global/global_effects.h
	src/common/global/global_objects.cpp
	src/common/global/global_objects.h
	src/common/global/global_skills.cpp
	src/common/global/global_skills.h
	src/common/global/global_utils.cpp
	src/common/global/global_utils.h
)

set(SOURCES_GUI_COMMON
	src/common/gui/gui.cpp
	src/common/gui/gui.h
	src/common/gui/menu_window.cpp
	src/common/gui/menu_window.h
	src/common/gui/option.cpp
	src/common/gui/option.h
	src/common/gui/textbox.cpp
	src/common/gui/textbox.h
)

set(SOURCES_COMMON
	${SOURCES_GLOBAL_COMMON}
	${SOURCES_GUI_COMMON}
	src/common/common.cpp
	src/common/common.h
	src/common/common_bindings.cpp
	src/common/dialogue.cpp
	src/common/dialogue.h
)

##### Allacrost Map Editor
set(HEADERS_MAP_EDITOR
	src/editor/dialogs.h
	src/editor/editor.h
	src/editor/editor_utils.h
	src/editor/map_data.h
	src/editor/map_view.h
	src/editor/tile_context.h
	src/editor/tile_layer.h
	src/editor/tileset.h
	src/editor/tileset_editor.h
)

set(SOURCES_MAP_EDITOR
	src/editor/dialogs.cpp
	src/editor/dialogs.h
	src/editor/editor.cpp
	src/editor/editor.h
	src/editor/editor_main.cpp
	src/editor/editor_utils.cpp
	src/editor/editor_utils.h
	src/editor/map_data.cpp
	src/editor/map_data.h
	src/editor/map_view.cpp
	src/editor/map_view.h
	src/editor/tile_context.cpp
	src/editor/tile_context.h
	src/editor/tile_layer.cpp
	src/editor/tile_layer.h
	src/editor/tileset.cpp
	src/editor/tileset.h
	src/editor/tileset_editor.cpp
	src/editor/tileset_editor.h
)

##### Complete Sources (all files needed to build each binary)
set(SOURCES_ALLACROST_BIN
	${SOURCES_LUABIND}
	${SOURCES_ENGINE}
	${SOURCES_COMMON}
	${SOURCES_MODES}
	src/defs.h
	src/main.cpp
	src/main_options.cpp
	src/main_options.h
	src/utils.cpp
	src/utils.h
)
# Add icon.rc file to Windows build list
if(WIN32)
	set(SOURCES_ALLACROST_BIN ${SOURCES_ALLACROST_BIN} icon.rc)
endif()

set(SOURCES_EDITOR_BIN
	${SOURCES_LUABIND}
	${SOURCES_ENGINE}
	${SOURCES_MAP_EDITOR}
	src/defs.h
	src/utils.cpp
	src/utils.h
)


###############################################################################
# Gettext Translation File Compilation
###############################################################################
# TODO: Use macros available in the FindGettext module to update/compile translation files in txt/
# https://cmake.org/cmake/help/v3.5/module/FindGettext.html
# GETTEXT_PROCESS_POT_FILE(...)
# GETTEXT_CREATE_TRANSLATIONS(..)


###############################################################################
# Compiler Flags -- Library Links -- Build Commands
###############################################################################
##### Set compiler flags and options.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# TODO: no-deprecated is enabled here because of the use of auto_ptr in luabind. Remove this flag once the deprecated auto_ptr is replaced.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")

# If building a release, optimize code fully (O3). Development is not optimized (O0) to speed compilation time
if(CMAKE_BUILD_TYPE STREQUAL "release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "develop")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
else()
	message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

# If this is a develop build, add enable debugging features in the compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "develop")
	string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
	if(CMAKE_BUILD_TYPE_TOLOWER MATCHES develop)
		if(CMAKE_COMPILER_IS_GNUCXX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")
			message(STATUS "Enabled development debugging")
		endif()
	endif()
endif()

# Flags to set things like package name and version, and where the game data is located (datadir)
set(FLAGS "${FLAGS} -DPACKAGE=\\\"allacrost\\\"")
set(FLAGS "${FLAGS} -DPACKAGE_VERSION=\\\"${VERSION}\\\"")
set(FLAGS "${FLAGS} -DDATADIR=\\\"${FULL_PKG_DATADIR}/\\\"")
set(FLAGS "${FLAGS} -DLOCALEDIR=\\\"${FULL_LOCALEDIR}/\\\"")
set(FLAGS "${FLAGS} -DPKG_NAME=\\\"${PKG_NAME}/\\\"")

# Add invalid pch compilation warning for the game.
if(CMAKE_COMPILER_IS_GNUCXX AND USE_PCH)
	set(FLAGS "${FLAGS} -Winvalid-pch")
endif()

# cmake build files are placed in build/cmake/, but the generated binaries need to be in root
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

##### Build the allacrost executable
add_executable(allacrost WIN32 ${SOURCES_ALLACROST_BIN})
set_target_properties(allacrost PROPERTIES COMPILE_FLAGS "${FLAGS}")
target_include_directories(allacrost PUBLIC
	${ALLACROST_HEADER_DIRS}
	${Boost_INCLUDE_DIRS}
	${JPEG_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
	${SDL2_TTF_INCLUDE_DIR}
	${VORBISFILE_INCLUDE_DIR}
)
# Note: some library variables linked to below will be undefined if not needed for the system that the build is running on
target_link_libraries(allacrost
	${EXTRA_LIBRARIES}
	${ICONV_LIBRARIES}
	${INTERNAL_LIBRARIES}
	${JPEG_LIBRARIES}
	${LIBINTL_LIBRARIES}
	${LUA_LIBRARIES}
	${OPENAL_LIBRARY}
	${OPENGL_LIBRARIES}
	${PNG_LIBRARIES}
	${SDL2_LIBRARY}
	${SDL2_TTF_LIBRARY}
	${VORBISFILE_LIBRARIES}
	${X11_LIBRARIES}
)

##### Build the allacrost-editor executable
if(EDITOR)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	add_executable(allacrost-editor WIN32 ${SOURCES_EDITOR_BIN})
	# The line below ensures that allacrost-editor is only built after allacrost
	add_dependencies(allacrost-editor allacrost)
	set_target_properties(allacrost-editor PROPERTIES COMPILE_FLAGS "${FLAGS}")
	target_include_directories(allacrost-editor PUBLIC
		${ALLACROST_HEADER_DIRS}
		${EDITOR_HEADER_DIRS}
		${Boost_INCLUDE_DIRS}
		${JPEG_INCLUDE_DIR}
		${LUA_INCLUDE_DIR}
		${OPENAL_INCLUDE_DIR}
		${OPENGL_INCLUDE_DIR}
		${PNG_INCLUDE_DIR}
		${SDL2_INCLUDE_DIR}
		${SDL2_TTF_INCLUDE_DIR}
		${VORBISFILE_INCLUDE_DIR}
	)
	# Note: some library variables linked to below will be undefined if not needed for the system that the build is running on
	target_link_libraries(allacrost-editor
		${EXTRA_LIBRARIES}
		${ICONV_LIBRARIES}
		${INTERNAL_LIBRARIES}
		${LIBINTL_LIBRARIES}
		${JPEG_LIBRARIES}
		${LUA_LIBRARIES}
		${OPENAL_LIBRARY}
		${OPENGL_LIBRARIES}
		${PNG_LIBRARIES}
		${SDL2_LIBRARY}
		${SDL2_TTF_LIBRARY}
		${VORBISFILE_LIBRARIES}
		${X11_LIBRARIES}
	)
	qt5_use_modules(allacrost-editor Core Gui OpenGL)
endif()

###############################################################################
# Installation/Uninstallation Target Settings
###############################################################################
install(TARGETS allacrost RUNTIME DESTINATION ${PKG_BINDIR})

##### CPack installation
If(UNIX)
	# Shortcut desktop file
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/allacrost.desktop"
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
	)
	# data files
	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
			DESTINATION ${PKG_DATADIR}
			FILES_MATCHING PATTERN "*.lua" PATTERN "*.png" PATTERN "*.ttf" PATTERN "*.wav" PATTERN "*.ogg"
	)
	# icon file
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/icons/program_icon_48x48.png"
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps RENAME allacrost.png
	)
	# KDE/Gnome app center app data
	install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/valyriatear.appdata.xml"
			DESTINATION ${CMAKE_INSTALL_PREFIX}/share/appdata
	)
endif()

set(CPACK_PACKAGE_NAME "allacrost")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hero of Allacrost")
set(CPACK_PACKAGE_VENDOR "The Allacrost Project")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/licenses.txt" )
set(CPACK_PACKAGE_INSTALL_DIRECTORY "allacrost")
set(CPACK_PACKAGE_VERSION_MAJOR ${VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VER_RELEASE})
if(WIN32)
	set(CPACK_NSIS_DISPLAY_NAME "Hero of Allacrost")
	set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/img/logos/program_icon.ico")
	set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/img/logos/program_icon.bmp")
	set(CPACK_NSIS_URL_INFO_ABOUT "https://allacrost.org")
endif()
include(CPack)

# OS X: Copy frameworks, resources etc files to their proper locations
if(APPLE)
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../dat DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../Resources/ FILES_MATCHING PATTERN "*.lua" PATTERN "*.png")
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../img DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../Resources/ FILES_MATCHING PATTERN "*.lua" PATTERN "*.png" PATTERN "*.ttf")
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../mus DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../Resources/ FILES_MATCHING PATTERN "*.ogg")
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../snd DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../Resources/ FILES_MATCHING PATTERN "*.wav" PATTERN "*.ogg")
	file(COPY ${SDL2_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY ${SDL2_TTF_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

##### Uninstall targets
# TODO: this block does not work, fix it
#configure_file(
#	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#	IMMEDIATE @ONLY
#)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Add Unix uninstall target
if(UNIX)
	  add_custom_target(
		  uninstall-allacrost
		  COMMAND rm -rf ${PKG_DATADIR}
		  COMMAND rm -f "${PKG_BINDIR}/allacrost"
		  VERBATIM
	  )
endif()

