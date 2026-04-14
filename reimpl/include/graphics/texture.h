/* texture.h - Texture loading matching original game's Texture_Create (0x476770) */
#ifndef TEXTURE_H
#define TEXTURE_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "graphics/gl_loader.h"
#include <stdint.h>

typedef struct {
    GLuint gl_id;       /* OpenGL texture ID */
    int width;
    int height;
    int channels;
    char filename[256]; /* Original filename (without extension) */
    int refcount;
} texture_t;

/* Initialize texture system with the Textures/ directory path */
void texture_system_init(const char *textures_dir);

/* Load texture matching original's lookup order:
   1. Try {name}-mip1.{fmt} then {name}.{fmt}
   2. Try PNG, then JPG, then BMP
   Returns cached texture if already loaded (increments refcount) */
texture_t *texture_load(const char *name);

/* Release a texture (decrements refcount, frees when 0) */
void texture_release(texture_t *tex);

/* Bind texture to GL unit */
void texture_bind(texture_t *tex, int unit);

/* Get texture by filename (cache lookup) */
texture_t *texture_find(const char *name);

/* Cleanup all textures */
void texture_system_shutdown(void);

#endif