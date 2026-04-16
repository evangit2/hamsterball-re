/* texture.h - Texture loading for D3D8 reimplementation
 * Matches original game's Texture_Create (0x476770) lookup pattern
 */
#ifndef TEXTURE_H
#define TEXTURE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>
#include <stdint.h>

#define MAX_TEXTURES 256

typedef struct {
    IDirect3DTexture8 *d3d_tex;  /* D3D8 texture surface */
    int width;
    int height;
    char filename[256];          /* Original filename (e.g. "PinkChecker.bmp") */
    int refcount;
} texture_t;

/* Initialize texture system */
void texture_system_init(IDirect3DDevice8 *device, const char *textures_dir);

/* Load texture by exact filename (e.g. "PinkChecker.bmp") 
   Returns cached texture if already loaded */
texture_t *texture_load(const char *filename);

/* Bind texture to stage 0 */
void texture_bind(texture_t *tex, IDirect3DDevice8 *device);

/* Release all textures */
void texture_system_shutdown(void);

#endif