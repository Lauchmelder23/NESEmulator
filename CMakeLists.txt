cmake_minimum_required (VERSION 3.8)

project ("NES Emulator")

find_package (SDL2)
if (SDL2_FOUND)
	message (STATUS "SDL2 found, using system-installed SDL2")
else (SDL2_FOUND)
	message (STATUS "SDL2 not found, using vendored static SDL2")
	set (BUILD_SHARED_LIBS OFF)
	add_subdirectory ("vendor/SDL")
	set (SDL2_INCLUDE_DIRS SDL2-static)
	set (SDL2_LIBRARIES SDL2-static SDL2main)
endif (SDL2_FOUND)

add_subdirectory ("NES Emulator")
