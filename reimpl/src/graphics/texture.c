/* texture.c - Texture loading
 * D3D8 build: loads PNG/BMP/TGA via stb_image, creates D3D8 textures manually
 * OpenGL build: loads via stb_image + OpenGL textures
 * No D3DX dependency — avoids d3dx8.lib linking issues with MinGW.
 * Matches original game's Texture_Create (0x476770) lookup pattern.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphics/texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <util/stb_image.h>

static struct {
    char textures_dir[512];
    texture_t textures[MAX_TEXTURES];
    int count;
#ifdef _D3D8
    IDirect3DDevice8 *device;
#endif
} g_tex = {0};

/* Try building a full path from filename */
static int build_path(char *out, size_t outsize, const char *filename) {
    if (!filename || !filename[0]) return 0;
    if (strchr(filename, '/') || strchr(filename, '\\')) {
        snprintf(out, outsize, "%s", filename);
        return 1;
    }
    if (g_tex.textures_dir[0]) {
        snprintf(out, outsize, "%s/%s", g_tex.textures_dir, filename);
        return 1;
    }
    return 0;
}

/* Load image file via stb_image, returns allocated RGBA buffer */
static unsigned char *load_image_pixels(const char *path, int *out_w, int *out_h) {
    int w, h, n;
    unsigned char *pixels = stbi_load(path, &w, &h, &n, 4);  /* Force RGBA */
    if (pixels) {
        *out_w = w;
        *out_h = h;
        return pixels;
    }
    
    /* Try alternative extensions */
    if (g_tex.textures_dir[0]) {
        /* Extract base name from path */
        char dir[512], base[256];
        strncpy(dir, path, sizeof(dir) - 1);
        char *last_slash = strrchr(dir, '/');
        if (last_slash) {
            *last_slash = '\0';
            strncpy(base, last_slash + 1, sizeof(base) - 1);
        } else {
            strncpy(base, path, sizeof(base) - 1);
            dir[0] = '\0';
        }
        char *dot = strrchr(base, '.');
        if (dot) *dot = '\0';
        
        const char *exts[] = {"png", "bmp", "jpg", "tga"};
        for (int i = 0; i < 4; i++) {
            char try_path[768];
            if (dir[0]) {
                snprintf(try_path, sizeof(try_path), "%s/%s.%s", dir, base, exts[i]);
            } else {
                snprintf(try_path, sizeof(try_path), "%s.%s", base, exts[i]);
            }
            pixels = stbi_load(try_path, &w, &h, &n, 4);
            if (pixels) {
                *out_w = w;
                *out_h = h;
                return pixels;
            }
        }
    }
    
    return NULL;
}

/* ============================================
 * D3D8 Implementation (stb_image + manual D3D texture)
 * ============================================ */
#ifdef _D3D8

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>

void texture_set_device(void *dev) {
    g_tex.device = (IDirect3DDevice8 *)dev;
}

static IDirect3DTexture8 *create_d3d_texture(int width, int height, const unsigned char *rgba_data) {
    if (!g_tex.device) return NULL;
    
    /* Power-of-2 size for D3D8 compatibility */
    int tw = 1, th = 1;
    while (tw < width) tw *= 2;
    while (th < height) th *= 2;
    
    IDirect3DTexture8 *tex = NULL;
    HRESULT hr = g_tex.device->lpVtbl->CreateTexture(g_tex.device, tw, th, 1, 0,
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
    
    /* Copy RGBA data into D3D texture (A8R8G8B8 = ARGB in native DWORD order on x86 LE) */
    DWORD *dst = (DWORD *)locked.pBits;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int si = (y * width + x) * 4;
            unsigned char r = rgba_data[si + 0];
            unsigned char g = rgba_data[si + 1];
            unsigned char b = rgba_data[si + 2];
            unsigned char a = rgba_data[si + 3];
            dst[y * (locked.Pitch / 4) + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    
    tex->lpVtbl->UnlockRect(tex, 0);
    return tex;
}

void texture_system_init(const char *textures_dir) {
    /* Preserve g_tex.device across init (set by texture_set_device before this call).
     * memset would zero the D3D device pointer, breaking all texture creation. */
    g_tex.count = 0;
    if (textures_dir) {
        strncpy(g_tex.textures_dir, textures_dir, sizeof(g_tex.textures_dir) - 1);
    }
}

texture_t *texture_load(const char *filename) {
    if (!filename || !filename[0]) return NULL;
    
    /* Check cache */
    for (int i = 0; i < g_tex.count; i++) {
        if (strcasecmp(g_tex.textures[i].filename, filename) == 0) {
            g_tex.textures[i].refcount++;
            return &g_tex.textures[i];
        }
    }
    
    char path[768];
    if (!build_path(path, sizeof(path), filename)) return NULL;
    
    int w, h;
    unsigned char *pixels = load_image_pixels(path, &w, &h);
    if (!pixels) {
        printf("[Texture] WARNING: Could not load: %s\n", filename);
        return NULL;
    }
    
    IDirect3DTexture8 *d3d_tex = create_d3d_texture(w, h, pixels);
    stbi_image_free(pixels);
    
    if (!d3d_tex) return NULL;
    
    /* Cache */
    if (g_tex.count >= MAX_TEXTURES) { d3d_tex->lpVtbl->Release(d3d_tex); return NULL; }
    texture_t *tex = &g_tex.textures[g_tex.count++];
    tex->d3d_tex = d3d_tex;
    tex->width = w;
    tex->height = h;
    tex->refcount = 1;
    tex->loaded = true;
    strncpy(tex->filename, filename, sizeof(tex->filename) - 1);
    
    printf("[Texture] Loaded: %s (%dx%d, D3D8)\n", filename, w, h);
    return tex;
}

void texture_bind(texture_t *tex, int stage) {
    if (!g_tex.device) return;
    g_tex.device->lpVtbl->SetTexture(g_tex.device, stage, 
        (tex && tex->loaded && tex->d3d_tex) ? (IDirect3DBaseTexture8*)tex->d3d_tex : NULL);
}

void texture_system_shutdown(void) {
    for (int i = 0; i < g_tex.count; i++) {
        if (g_tex.textures[i].d3d_tex) {
            g_tex.textures[i].d3d_tex->lpVtbl->Release(g_tex.textures[i].d3d_tex);
        }
    }
    memset(&g_tex, 0, sizeof(g_tex));
}

/* ============================================
 * OpenGL Implementation
 * ============================================ */
#else

#include <GL/glew.h>

static texture_t *load_gl_texture(const char *filename) {
    if (!filename || !filename[0]) return NULL;
    
    char path[768];
    const char *load_path = NULL;
    
    if (strchr(filename, '/') || strchr(filename, '\\')) {
        load_path = filename;
    } else if (g_tex.textures_dir[0]) {
        snprintf(path, sizeof(path), "%s/%s", g_tex.textures_dir, filename);
        load_path = path;
    }
    
    if (!load_path) return NULL;
    
    int w, h;
    unsigned char *pixels = load_image_pixels(load_path, &w, &h);
    if (!pixels) {
        printf("[Texture] WARNING: Could not load: %s\n", filename);
        return NULL;
    }
    
    /* Create OpenGL texture */
    GLuint gl_tex;
    glGenTextures(1, &gl_tex);
    glBindTexture(GL_TEXTURE_2D, gl_tex);
    
    int tw = 1, th = 1;
    while (tw < w) tw *= 2;
    while (th < h) th *= 2;
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
    
    if (g_tex.count >= MAX_TEXTURES) return NULL;
    texture_t *tex = &g_tex.textures[g_tex.count++];
    tex->gl_tex = gl_tex;
    tex->width = w;
    tex->height = h;
    tex->refcount = 1;
    tex->loaded = true;
    strncpy(tex->filename, filename, sizeof(tex->filename) - 1);
    
    printf("[Texture] Loaded: %s (%dx%d, GL tex %u)\n", filename, w, h, gl_tex);
    return tex;
}

void texture_system_init(const char *textures_dir) {
    /* Preserve g_tex.device across init (set by texture_set_device before this call).
     * memset would zero the D3D device pointer, breaking all texture creation. */
    g_tex.count = 0;
    if (textures_dir) {
        strncpy(g_tex.textures_dir, textures_dir, sizeof(g_tex.textures_dir) - 1);
    }
}

texture_t *texture_load(const char *filename) {
    if (!filename || !filename[0]) return NULL;
    
    /* Check cache */
    for (int i = 0; i < g_tex.count; i++) {
        if (strcasecmp(g_tex.textures[i].filename, filename) == 0) {
            g_tex.textures[i].refcount++;
            return &g_tex.textures[i];
        }
    }
    
    return load_gl_texture(filename);
}

void texture_bind(texture_t *tex, int stage) {
    glActiveTexture(GL_TEXTURE0 + stage);
    if (tex && tex->loaded && tex->gl_tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex->gl_tex);
    } else {
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

void texture_system_shutdown(void) {
    for (int i = 0; i < g_tex.count; i++) {
        if (g_tex.textures[i].gl_tex) {
            glDeleteTextures(1, &g_tex.textures[i].gl_tex);
        }
    }
    memset(&g_tex, 0, sizeof(g_tex));
}

#endif /* _D3D8 */