/*
 * Hamsterball OpenGL Renderer
 * Based on Graphics_Initialize at 0x455380
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "graphics/gl_loader.h"
#include "physics/physics.h"

bool renderer_init(int width, int height);
void renderer_shutdown(void);
void renderer_clear(void);
void renderer_present(void);

/* Camera setup */
void renderer_set_camera(vec3_t pos, vec3_t target);

/* Window access (for event handling) */
SDL_Window *renderer_get_window(void);

/* Resize handling */
void renderer_resize(int width, int height);

/* Debug drawing helpers */
void renderer_draw_sphere(float x, float y, float z, float radius,
                          float r, float g, float b, float a);

/* Texture management (future) */
void* renderer_create_texture(const char *path);
void renderer_destroy_texture(void *texture);
void renderer_set_texture(void *texture);

/* Font rendering (future) */
void renderer_draw_text(const char *text, float x, float y, float scale);

#endif /* RENDERER_H */