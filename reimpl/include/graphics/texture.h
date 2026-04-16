/* texture.h - Texture loading for OpenGL reimplementation
 * Matches original game's Texture_Create (0x476770) lookup pattern
 * Uses SDL2_image + OpenGL instead of D3D8
 */
#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stdbool.h>
#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TEXTURES 256

typedef struct {
    GLuint gl_tex;          /* OpenGL texture ID */
    int width;
    int height;
    char filename[256];     /* Original filename (e.g. "PinkChecker.bmp") */
    int refcount;
    bool loaded;
} texture_t;

/* Initialize texture system. textures_dir = path to Textures/ folder */
void texture_system_init(const char *textures_dir);

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