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

# Source files - exclude test files, level_viewer (has its own main), and Windows-only
file(GLOB_RECURSE SOURCES "src/*.c")
list(FILTER SOURCES EXCLUDE REGEX "level_viewer\\.c$")
list(FILTER SOURCES EXCLUDE REGEX "level_viewer_v2\\.c$")
list(FILTER SOURCES EXCLUDE REGEX "mesh_test\\.c$")
list(FILTER SOURCES EXCLUDE REGEX "win32_main\\.c$")  # Windows-only D3D8 main
list(FILTER SOURCES EXCLUDE REGEX "test_parser\\.c$")  # Standalone test, uses old API

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})

# Build the level viewer (separate target)
add_executable(level_viewer_v2 src/level/level_viewer_v2.c src/level/meshworld_parser.c src/level/mesh_parser.c)
target_link_libraries(level_viewer_v2
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    m
)
target_include_directories(level_viewer_v2 PRIVATE ${CMAKE_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS})

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
target_include_directories(hamsterball PRIVATE ${CMAKE_SOURCE_DIR}/include ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

# ==================== Tests ====================
enable_testing()

# Common test dependencies: filesys + mesh parsers
set(TEST_DEPS
    src/core/filesys.c
    src/level/meshworld_parser.c
    src/level/mesh_parser.c
)

# Unit test: Quick sanity tests
add_executable(test_quick tests/test_quick.c src/core/filesys.c src/level/meshworld_parser.c src/level/mesh_parser.c)
target_include_directories(test_quick PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(test_quick m)
add_test(NAME quick COMMAND test_quick)

# Unit test: MESH parser
add_executable(test_mesh tests/test_mesh.c src/level/mesh_parser.c)
target_include_directories(test_mesh PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(test_mesh m)
add_test(NAME mesh COMMAND test_mesh)

# Unit test: Asset compatibility
add_executable(test_assets tests/test_assets.c src/core/filesys.c src/level/meshworld_parser.c src/level/mesh_parser.c)
target_include_directories(test_assets PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(test_assets m)
add_test(NAME assets COMMAND test_assets)

# Unit test: XML parsers (jukebox + race data)
add_executable(test_xml tests/test_xml_parsers.c src/audio/jukebox.c src/game/race_data.c)
target_include_directories(test_xml PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(test_xml m)
add_test(NAME xml COMMAND test_xml)

# Install targets
install(TARGETS hamsterball level_viewer_v2 DESTINATION bin)