cmake_minimum_required(VERSION 3.20)
project(hamsterball-re C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use pkg-config for SDL2 (more portable)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Source files - exclude level_viewer.c (has its own main)
file(GLOB_RECURSE SOURCES "src/*.c")
list(FILTER SOURCES EXCLUDE REGEX "level_viewer\\.c$")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})

# Build the level viewer (separate target)
add_executable(level_viewer src/level/level_viewer.c src/level/meshworld_parser.c)
target_link_libraries(level_viewer
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    m
)
target_include_directories(level_viewer PRIVATE ${CMAKE_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS})

# Build the main game (all sources)
add_executable(hamsterball ${SOURCES})
target_link_libraries(hamsterball
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    m
)
target_include_directories(hamsterball PRIVATE ${CMAKE_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS})

# Install targets
install(TARGETS hamsterball level_viewer DESTINATION bin)