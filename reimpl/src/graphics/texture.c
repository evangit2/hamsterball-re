/* texture.c - OpenGL texture loading using stb_image
 * Matches original game's Texture_Create (0x476770) lookup pattern
 * Uses SDL2_image + OpenGL instead of D3D8
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <util/stb_image.h>

#include <GL/glew.h>

#include <graphics/texture.h>

static struct {
    char textures_dir[512];
    texture_t textures[MAX_TEXTURES];
    int count;
} g_tex = {0};

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
    
    /* Create OpenGL texture from pixel data */
    GLuint gl_tex;
    glGenTextures(1, &gl_tex);
    glBindTexture(GL_TEXTURE_2D, gl_tex);
    
    /* Power-of-2 size (for compatibility) */
    int tw = 1, th = 1;
    while (tw < w) tw *= 2;
    while (th < h) th *= 2;
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    /* Generate mipmaps */
    glGenerateMipmap(GL_TEXTURE_2D);
    
    /* Default parameters: wrap + linear filtering */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
    
    /* Cache */
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

/* Initialize */
void texture_system_init(const char *textures_dir) {
    memset(&g_tex, 0, sizeof(g_tex));
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

/* Bind texture for rendering */
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

/* Shutdown */
void texture_system_shutdown(void) {
    for (int i = 0; i < g_tex.count; i++) {
        if (g_tex.textures[i].gl_tex) {
            glDeleteTextures(1, &g_tex.textures[i].gl_tex);
        }
    }
    memset(&g_tex, 0, sizeof(g_tex));
}