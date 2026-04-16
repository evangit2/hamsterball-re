/* texture.c - D3D8 texture loading using stb_image
 * No D3DX dependency — loads PNG/BMP/TGA/JPG via stb_image,
 * creates D3D8 textures manually.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <util/stb_image.h>

#include <graphics/texture.h>

#define MAX_TEXTURES 256

static struct {
    IDirect3DDevice8 *device;
    char textures_dir[512];
    texture_t textures[MAX_TEXTURES];
    int count;
} g_tex = {0};

/* Create a D3D8 texture from raw RGBA pixel data */
static IDirect3DTexture8 *create_texture_from_pixels(IDirect3DDevice8 *device, 
                                                        int width, int height, 
                                                        const unsigned char *rgba_data) {
    IDirect3DTexture8 *tex = NULL;
    HRESULT hr;
    
    /* D3DXCreateTextureFromFile uses power-of-2 textures, so should we */
    int tw = 1, th = 1;
    while (tw < width) tw *= 2;
    while (th < height) th *= 2;
    
    hr = device->lpVtbl->CreateTexture(device, tw, th, 1, 0, 
                                         D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex);
    if (FAILED(hr)) {
        printf("[Texture] CreateTexture failed: 0x%08lx (size %dx%d)\n", hr, tw, th);
        return NULL;
    }
    
    D3DLOCKED_RECT locked;
    hr = tex->lpVtbl->LockRect(tex, 0, &locked, NULL, 0);
    if (FAILED(hr)) {
        printf("[Texture] LockRect failed: 0x%08lx\n", hr);
        tex->lpVtbl->Release(tex);
        return NULL;
    }
    
    /* Copy RGBA data into D3D texture (A8R8G8B8 format = native DWORD order) */
    DWORD *dst = (DWORD *)locked.pBits;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int si = (y * width + x) * 4;  /* Source index in RGBA data */
            unsigned char r = rgba_data[si + 0];
            unsigned char g = rgba_data[si + 1];
            unsigned char b = rgba_data[si + 2];
            unsigned char a = rgba_data[si + 3];
            /* D3DFMT_A8R8G8B8 is stored as ARGB in native byte order on x86 (little-endian) */
            dst[y * (locked.Pitch / 4) + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    
    tex->lpVtbl->UnlockRect(tex, 0);
    return tex;
}

/* Try loading a texture from various paths */
static texture_t *load_image_file(const char *filename) {
    if (!filename || !filename[0]) return NULL;
    
    char path[768];
    const char *load_path = NULL;
    
    /* Try direct path first */
    if (strchr(filename, '/') || strchr(filename, '\\')) {
        load_path = filename;
    } else if (g_tex.textures_dir[0]) {
        snprintf(path, sizeof(path), "%s/%s", g_tex.textures_dir, filename);
        load_path = path;
    }
    
    if (!load_path) return NULL;
    
    /* Load with stb_image */
    int w, h, n;
    unsigned char *pixels = stbi_load(load_path, &w, &h, &n, 4);  /* Force RGBA */
    if (!pixels) {
        /* Try alternative extensions */
        if (g_tex.textures_dir[0]) {
            char base[256];
            strncpy(base, filename, sizeof(base) - 1);
            char *dot = strrchr(base, '.');
            if (dot) *dot = '\0';
            
            const char *exts[] = {"png", "bmp", "jpg", "tga"};
            for (int i = 0; i < 4 && !pixels; i++) {
                snprintf(path, sizeof(path), "%s/%s.%s", g_tex.textures_dir, base, exts[i]);
                pixels = stbi_load(path, &w, &h, &n, 4);
                if (pixels) load_path = path;
            }
        }
    }
    
    if (!pixels) {
        printf("[Texture] WARNING: Could not load: %s\n", filename);
        return NULL;
    }
    
    /* Create D3D8 texture from pixel data */
    IDirect3DTexture8 *d3d_tex = create_texture_from_pixels(g_tex.device, w, h, pixels);
    stbi_image_free(pixels);
    
    if (!d3d_tex) return NULL;
    
    /* Cache */
    if (g_tex.count >= MAX_TEXTURES) return NULL;
    texture_t *tex = &g_tex.textures[g_tex.count++];
    tex->d3d_tex = d3d_tex;
    tex->width = w;
    tex->height = h;
    tex->refcount = 1;
    strncpy(tex->filename, filename, sizeof(tex->filename) - 1);
    
    printf("[Texture] Loaded: %s (%dx%d)\n", filename, w, h);
    return tex;
}

/* Initialize */
void texture_system_init(IDirect3DDevice8 *device, const char *textures_dir) {
    memset(&g_tex, 0, sizeof(g_tex));
    g_tex.device = device;
    if (textures_dir) {
        strncpy(g_tex.textures_dir, textures_dir, sizeof(g_tex.textures_dir) - 1);
    }
}

/* Load texture by filename (with caching) */
texture_t *texture_load(const char *filename) {
    if (!filename || !filename[0]) return NULL;
    
    /* Check cache */
    for (int i = 0; i < g_tex.count; i++) {
        if (strcasecmp(g_tex.textures[i].filename, filename) == 0) {
            g_tex.textures[i].refcount++;
            return &g_tex.textures[i];
        }
    }
    
    return load_image_file(filename);
}

/* Bind texture to device */
void texture_bind(texture_t *tex, IDirect3DDevice8 *device) {
    if (tex && tex->d3d_tex) {
        device->lpVtbl->SetTexture(device, 0, (IDirect3DBaseTexture8 *)tex->d3d_tex);
    } else {
        device->lpVtbl->SetTexture(device, 0, NULL);
    }
}

/* Shutdown */
void texture_system_shutdown(void) {
    for (int i = 0; i < g_tex.count; i++) {
        if (g_tex.textures[i].d3d_tex) {
            g_tex.textures[i].d3d_tex->lpVtbl->Release(g_tex.textures[i].d3d_tex);
        }
    }
    memset(&g_tex, 0, sizeof(g_tex));
}