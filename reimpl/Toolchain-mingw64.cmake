# Cross-compile toolchain for Windows x64
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# SDL2 + mingw libs location
set(MINGW_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/mingw-libs")

set(SDL2_DIR "${MINGW_LIBS}/SDL2-2.32.0/x86_64-w64-mingw32")
set(SDL2_IMAGE_DIR "${MINGW_LIBS}/SDL2_image-2.8.4/x86_64-w64-mingw32")
set(SDL2_MIXER_DIR "${MINGW_LIBS}/SDL2_mixer-2.8.0/x86_64-w64-mingw32")

set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/include/SDL2")
set(SDL2_IMAGE_INCLUDE_DIRS "${SDL2_IMAGE_DIR}/include/SDL2")
set(SDL2_MIXER_INCLUDE_DIRS "${SDL2_MIXER_DIR}/include/SDL2")

set(SDL2_LIBRARIES "-L${SDL2_DIR}/lib -lSDL2main -lSDL2")
set(SDL2_IMAGE_LIBRARIES "-L${SDL2_IMAGE_DIR}/lib -lSDL2_image")
set(SDL2_MIXER_LIBRARIES "-L${SDL2_MIXER_DIR}/lib -lSDL2_mixer")

# OpenGL is provided by mingw
set(OPENGL_LIBRARIES "-lopengl32 -lglu32")
set(GLEW_LIBRARIES "")

# Static linking
set(CMAKE_EXE_LINKER_FLAGS "-static -mwindows")

# Don't search host system paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)