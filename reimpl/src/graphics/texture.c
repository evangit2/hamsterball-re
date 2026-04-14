/* texture.c - Texture loading via SDL_image, matching original's Texture_Create flow
 * Original at 0x476770 (Texture_Create):
 *   1. Strip extension from filename
 *   2. Try: {path}{name}-mip1.{fmt}, then {path}{name}.{fmt}  
 *   3. Try formats: PNG, JPG, BMP
 *   4. Cache in Graphics+0x2E4 AthenaList
 */
#include "graphics/texture.h"
#include "core/filesys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TEXTURES 512
static texture_t g_textures[MAX_TEXTURES];
static int g_texture_count = 0;
static char g_textures_dir[512] = "";

void texture_system_init(const char *textures_dir) {
    strncpy(g_textures_dir, textures_dir, sizeof(g_textures_dir) - 1);
    g_texture_count = 0;
    memset(g_textures, 0, sizeof(g_textures));
}

texture_t *texture_find(const char *name) {
    for (int i = 0; i < g_texture_count; i++) {
        if (strcmp(g_textures[i].filename, name) == 0) {
            return &g_textures[i];
        }
    }
    return NULL;
}

static texture_t *alloc_texture(void) {
    if (g_texture_count >= MAX_TEXTURES) return NULL;
    texture_t *t = &g_textures[g_texture_count++];
    memset(t, 0, sizeof(*t));
    t->gl_id = 0;
    t->refcount = 1;
    return t;
}

/* Try loading from a specific path */
static SDL_Surface *try_load_image(const char *path) {
    if (!filesys_file_exists(path)) return NULL;
    return IMG_Load(path);
}

texture_t *texture_load(const char *name) {
    if (!name || !name[0]) return NULL;
    
    /* Strip extension if given */
    char basename[256];
    strncpy(basename, name, sizeof(basename) - 1);
    basename[sizeof(basename) - 1] = '\0';
    char *dot = strrchr(basename, '.');
    if (dot && (strcmp(dot, ".png") == 0 || strcmp(dot, ".jpg") == 0 ||
                strcmp(dot, ".bmp") == 0 || strcmp(dot, ".PNG") == 0 ||
                strcmp(dot, ".JPG") == 0 || strcmp(dot, ".BMP") == 0)) {
        *dot = '\0';
    }
    
    /* Cache check */
    texture_t *cached = texture_find(basename);
    if (cached) {
        cached->refcount++;
        return cached;
    }
    
    /* Original tries: {dir}{name}-mip1.{fmt} then {dir}{name}.{fmt}
     * For each: PNG, JPG, BMP */
    const char *formats[] = {"png", "jpg", "bmp", NULL};
    SDL_Surface *surface = NULL;
    char path[512];
    
    for (int f = 0; formats[f] && !surface; f++) {
        /* Try with -mip1 suffix first */
        snprintf(path, sizeof(path), "%s/%s-mip1.%s", g_textures_dir, basename, formats[f]);
        surface = try_load_image(path);
        
        if (!surface) {
            /* Try without -mip1 */
            snprintf(path, sizeof(path), "%s/%s.%s", g_textures_dir, basename, formats[f]);
            surface = try_load_image(path);
        }
    }
    
    if (!surface) return NULL;
    
    texture_t *tex = alloc_texture();
    if (!tex) { SDL_FreeSurface(surface); return NULL; }
    
    strncpy(tex->filename, basename, sizeof(tex->filename) - 1);
    tex->width = surface->w;
    tex->height = surface->h;
    tex->channels = surface->format->BytesPerPixel;
    
    /* Convert to RGBA if needed (OpenGL prefers RGBA) */
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba) return tex;
    
    /* Upload to OpenGL */
    glGenTextures(1, &tex->gl_id);
    glBindTexture(GL_TEXTURE_2D, tex->gl_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
    
    /* Generate mipmaps */
    glGenerateMipmap(GL_TEXTURE_2D);
    
    SDL_FreeSurface(rgba);
    return tex;
}

void texture_release(texture_t *tex) {
    if (!tex) return;
    tex->refcount--;
    if (tex->refcount <= 0 && tex->gl_id) {
        glDeleteTextures(1, &tex->gl_id);
        tex->gl_id = 0;
        tex->filename[0] = '\0';
    }
}

void texture_bind(texture_t *tex, int unit) {
    if (!tex || !tex->gl_id) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex->gl_id);
}

void texture_system_shutdown(void) {
    for (int i = 0; i < g_texture_count; i++) {
        if (g_textures[i].gl_id) {
            glDeleteTextures(1, &g_textures[i].gl_id);
        }
    }
    g_texture_count = 0;
}