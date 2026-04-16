/* texture.h - Texture loading
 * Matches original game's Texture_Create (0x476770) lookup pattern
 *
 * D3D8 build: uses IDirect3DTexture8* (built with _D3D8 defined)
 * OpenGL build: uses GLuint texture IDs
 */
#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _D3D8
/* Win32/D3D8 build — forward declare D3D8 types */
#define WIN32_LEAN_AND_MEAN
#include <d3d8.h>
typedef struct {
    IDirect3DTexture8 *d3d_tex;  /* D3D8 texture interface */
    int width;
    int height;
    char filename[256];
    int refcount;
    bool loaded;
} texture_t;
#else
/* OpenGL/SDL2 build */
#include <GL/glew.h>
typedef struct {
    GLuint gl_tex;          /* OpenGL texture ID */
    int width;
    int height;
    char filename[256];
    int refcount;
    bool loaded;
} texture_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TEXTURES 256

/* Initialize texture system. textures_dir = path to Textures/ folder */
void texture_system_init(const char *textures_dir);

#ifdef _D3D8
/* Set D3D8 device for texture creation (call after device is created) */
void texture_set_device(void *device);
#endif

/* Load texture by exact filename (e.g. "PinkChecker.bmp")
   Returns cached texture if already loaded */
texture_t *texture_load(const char *filename);

/* Bind texture to stage 0 for rendering */
void texture_bind(texture_t *tex, int stage);

/* Release all textures */
void texture_system_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif