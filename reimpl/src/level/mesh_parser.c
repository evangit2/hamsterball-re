/*
 * Hamsterball MESH Binary Parser v1
 * Parses .MESH 3D model files
 */

#include "level/mesh_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* Read helpers */
static uint32_t read_u32(const uint8_t *data, size_t pos) {
    return *(uint32_t *)(data + pos);
}

static float read_f32(const uint8_t *data, size_t pos) {
    return *(float *)(data + pos);
}

mesh_model_t *mesh_parse(const uint8_t *data, size_t size) {
    if (!data || size < 32) return NULL;
    
    mesh_model_t *model = calloc(1, sizeof(mesh_model_t));
    if (!model) return NULL;
    
    size_t pos = 0;
    
    /* Header: version */
    uint32_t version = read_u32(data, pos); pos += 4;
    if (version < 1 || version > 5) {
        fprintf(stderr, "MESH: unsupported version %u\n", version);
        free(model);
        return NULL;
    }
    model->material.ambient[3] = 1.0f; /* default alpha */
    model->material.diffuse[3] = 1.0f;
    model->material.specular[3] = 1.0f;
    
    /* Name */
    uint32_t name_len = read_u32(data, pos); pos += 4;
    if (name_len < 256) {
        memcpy(model->name, data + pos, name_len);
        model->name[name_len] = '\0';
    }
    pos += name_len;
    
    if (version == 1) {
        /* Version 1: 88 bytes of material, then texture, then geometry */
        for (int i = 0; i < 4; i++) model->material.ambient[i] = read_f32(data, pos + i*4);
        for (int i = 0; i < 4; i++) model->material.diffuse[i] = read_f32(data, pos + 16 + i*4);
        for (int i = 0; i < 4; i++) model->material.specular[i] = read_f32(data, pos + 32 + i*4);
        model->material.shine = read_f32(data, pos + 48);
        model->material.shadow_bias = read_f32(data, pos + 52);
        model->material.shadow_scale = read_f32(data, pos + 56);
        pos += 88;
        
        /* Texture name */
        if (pos + 4 > size) { mesh_free(model); return NULL; }
        uint32_t tex_len = read_u32(data, pos); pos += 4;
        if (tex_len < 256 && pos + tex_len <= size) {
            memcpy(model->material.texture, data + pos, tex_len);
            model->material.texture[tex_len] = '\0';
        }
        pos += tex_len;
    } else {
        /* Version 2-5: Transform matrix + sub-object headers, then multiple mesh buffers */
        /* All versions 2-5 share the same header structure */
        /* Skip sub-object count and transform matrix etc - we'll parse geometry later */
        /* For now, skip to texture string */
        size_t tex_search_start = pos;
        
        /* Versions 2-5 have: position(12 bytes), material(88), sub_count, then sub-objects
           with name, texture, transforms, etc. The format is complex but we can find texture strings. */
        
        /* Scan for texture string (.png or .jpg) */
        int found_tex = 0;
        for (size_t p = tex_search_start; p < size - 4 && p < tex_search_start + 500; p++) {
            uint32_t slen = read_u32(data, p);
            if (p + 4 + slen > size || slen > 100) continue;
            char candidate[256] = {0};
            if (slen < 256) memcpy(candidate, data + p + 4, slen);
            /* Check if it looks like a texture filename */
            if (strstr(candidate, ".png") || strstr(candidate, ".jpg") || strstr(candidate, ".bmp")) {
                strncpy(model->material.texture, candidate, 255);
                pos = p + 4 + slen;
                found_tex = 1;
                break;
            }
        }
        if (!found_tex) {
            /* Try to skip past transform data (varies by version) */
            /* V2: 88 bytes material + count + unknowns */
            /* V3: 88 bytes material + count + position(12) */
            pos = 8 + name_len;
            /* Skip material data */
            pos += 88;
            /* Version-specific header data */
            if (version >= 3) pos += 12; /* position data */
        }
    }
    
    /* Vertex count and geometry data */
    if (pos + 4 > size) { mesh_free(model); return NULL; }
    
    /* For version 1: two uint32s (vertex_count, unknown) then vertices */
    /* For version 2+: may have different layout */
    if (version == 1) {
        model->vertex_count = read_u32(data, pos); pos += 4;
        pos += 4; /* skip unknown uint32 */
    } else {
        /* For versions 2-5, vertex/face data is embedded in sub-object sections
           Try to find vertex count by scanning for reasonable values */
        model->vertex_count = 0;
        /* Scan ahead for a small integer that could be vertex count */
        for (int i = 0; i < 20 && pos + 4 <= size; i++) {
            uint32_t val = read_u32(data, pos);
            if (val >= 3 && val < 10000) {
                /* Check if following data looks like vertex positions */
                float fx = read_f32(data, pos + 4);
                float fy = read_f32(data, pos + 8);
                float fz = read_f32(data, pos + 12);
                if (pos + 16 <= size && fabsf(fx) < 500 && fabsf(fy) < 500 && fabsf(fz) < 500) {
                    model->vertex_count = val;
                    pos += 4;
                    break;
                }
            }
            pos += 4;
        }
        if (model->vertex_count == 0) {
            /* Give up - return model with 0 vertices */
            return model;
        }
    }
    
    /* Allocate and read vertices */
    if (model->vertex_count > 0 && model->vertex_count < 100000) {
        model->vertices = calloc(model->vertex_count, sizeof(mesh_vertex_t));
        if (!model->vertices) { mesh_free(model); return NULL; }
        
        for (int i = 0; i < model->vertex_count && pos + 32 <= size; i++) {
            mesh_vertex_t *v = &model->vertices[i];
            v->x = read_f32(data, pos); pos += 4;
            v->y = read_f32(data, pos); pos += 4;
            v->z = read_f32(data, pos); pos += 4;
            v->nx = read_f32(data, pos); pos += 4;
            v->ny = read_f32(data, pos); pos += 4;
            v->nz = read_f32(data, pos); pos += 4;
            v->u = read_f32(data, pos); pos += 4;
            v->v = read_f32(data, pos); pos += 4;
        }
        
        /* Calculate bounding box */
        if (model->vertex_count > 0) {
            model->min_x = model->max_x = model->vertices[0].x;
            model->min_y = model->max_y = model->vertices[0].y;
            model->min_z = model->max_z = model->vertices[0].z;
            
            for (int i = 1; i < model->vertex_count; i++) {
                mesh_vertex_t *v = &model->vertices[i];
                if (v->x < model->min_x) model->min_x = v->x;
                if (v->x > model->max_x) model->max_x = v->x;
                if (v->y < model->min_y) model->min_y = v->y;
                if (v->y > model->max_y) model->max_y = v->y;
                if (v->z < model->min_z) model->min_z = v->z;
                if (v->z > model->max_z) model->max_z = v->z;
            }
        }
    }
    
    return model;
}

mesh_model_t *mesh_parse_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = malloc(size);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, size, f);
    fclose(f);
    
    mesh_model_t *model = mesh_parse(data, size);
    free(data);
    return model;
}

void mesh_free(mesh_model_t *model) {
    if (model) {
        free(model->vertices);
        free(model);
    }
}