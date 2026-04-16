/* texture.c - Texture loading via D3DXCreateTextureFromFileEx
 * Matches original's Texture_Create (0x476770):
 *   1. Strip extension from filename
 *   2. Try: {path}{name}-mip1.{fmt}, then {path}{name}.{fmt}  
 *   3. Try formats: PNG, JPG, BMP, TGA, DDS
 *   4. Cache in App+0x1C0 list
 * 
 * For MinGW we use D3DX8 which is part of the dx8 redist.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tchar.h>

#define CINTERFACE
#include <d3dx8.h>

#include <util/stb_image.h>
#include <util/stb_image_write.h>

#include <graphics/texture.h>

#define MAX_TEXTURES 512
static texture_t g_textures[MAX_TEXTURES];
static int g_texture_count = 0;
static char g_textures_dir[512] = {0};
static IDirect3DDevice8 *g_device = NULL;

/* ===== Path helpers ===== */
static void make_path(char *buf, size_t bufsize, const char *name, const char *suffix, const char *ext) {
    snprintf(buf, bufsize, \"%s/%s%s.%s\", g_textures_dir, name, suffix, ext);
}

/* Try loading from a specific path */
static SDL_Surface *try_load_image(const char *path) {
    /* Use stb_image */
    int w, h, n;
    unsigned char *pixels = stbi_load(path, &w, &h, &n, STBI_default);
    if (!pixels) return NULL;
    
    SDL_Surface *surf = SDL_CreateSurfaceFrom(path, w, h, 0);
    if (!surf) { free(pixels); return NULL; }
    return surf;
}