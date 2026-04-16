/*
 * Hamsterball Binary MeshWorld Parser v5
 * Based on OFFICIAL Raptisoft 3DS Max exporter source (MeshWorldExport.cpp)
 * 
 * Format definitive spec: docs/MESHWORLD_BINARY_FORMAT_OFFICIAL.md
 * Reference source: reference/raptisoft-exporter/MeshWorldExport/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "level/meshworld_parser.h"

/* ===== Reader helpers ===== */
typedef struct {
    const uint8_t *data;
    size_t size;
    size_t pos;
} mw_reader_t;

static int mw_read(mw_reader_t *r, void *buf, size_t count) {
    if (r->pos + count > r->size) return -1;
    memcpy(buf, r->data + r->pos, count);
    r->pos += count;
    return 0;
}

static uint32_t mw_read_u32(mw_reader_t *r) {
    uint32_t val;
    if (mw_read(r, &val, 4) < 0) return 0;
    return val;
}

static float mw_read_f32(mw_reader_t *r) {
    float val;
    if (mw_read(r, &val, 4) < 0) return 0.0f;
    return val;
}

/* Read a length-prefixed string (official format: [int length][char[] data]) */
static int mw_read_string(mw_reader_t *r, char *buf, int bufsize) {
    uint32_t len = mw_read_u32(r);
    if (len == 0 || len > (uint32_t)bufsize - 1) {
        if (len > 0 && len < bufsize) {
            /* Read but don't store oversized strings fully */
            mw_read(r, buf, len > (size_t)bufsize ? bufsize : len);
            buf[bufsize-1] = '\0';
            return 0;
        }
        return -1;
    }
    if (mw_read(r, buf, len) < 0) return -1;
    buf[len] = '\0';
    /* Remove trailing NUL if present */
    if (len > 0 && buf[len-1] == '\0') { /* already NUL-terminated */ }
    return 0;
}

/* ===== String type classification ===== */
static mw_obj_type_t classify_type(const char *s) {
    if (!s || s[0] == '\0') return MW_OBJ_UNKNOWN;
    if (strncmp(s, "START", 5) == 0) return MW_OBJ_START;
    if (strncmp(s, "FLAG", 4) == 0) return MW_OBJ_FLAG;
    if (strcmp(s, "SAFESPOT") == 0) return MW_OBJ_SAFESPOT;
    if (strncmp(s, "CameraLocus", 11) == 0 || strncmp(s, "CAMERALOOKAT", 12) == 0) return MW_OBJ_CAMERALOOKAT;
    if (strncmp(s, "PLATFORM", 8) == 0) return MW_OBJ_PLATFORM;
    if (strncmp(s, "N:SINKPLATFORM", 14) == 0) return MW_OBJ_SINKPLATFORM;
    if (strncmp(s, "N:", 2) == 0 || strncmp(s, "E:", 2) == 0 || strncmp(s, "S:", 2) == 0) return MW_OBJ_GOAL;
    if (strncmp(s, "RND", 3) == 0 || strncmp(s, "+RND", 4) == 0) return MW_OBJ_RND;
    if (strcmp(s, "STANDS") == 0) return MW_OBJ_PLATFORM;
    if (strcmp(s, "BADBALL") == 0) return MW_OBJ_BADBALL;
    if (strcmp(s, "BCMESH") == 0) return MW_OBJ_BCMBALL;
    if (strcmp(s, "BallPath") == 0) return MW_OBJ_BALLPATH;
    return MW_OBJ_UNKNOWN;
}

/* ===== Object addition ===== */
static void add_object(mw_level_t *level, const char *type_str,
                       float d3d_x, float d3d_y, float d3d_z,
                       float rx, float ry, float rz) {
    if (level->object_count >= level->object_capacity) {
        level->object_capacity *= 2;
        level->objects = realloc(level->objects, level->object_capacity * sizeof(mw_object_t));
    }
    
    mw_object_t *obj = &level->objects[level->object_count++];
    memset(obj, 0, sizeof(*obj));
    
    obj->type = classify_type(type_str);
    strncpy(obj->type_string, type_str, sizeof(obj->type_string) - 1);
    
    /* Positions already converted to D3D x,y,z by caller */
    obj->position.x = d3d_x;
    obj->position.y = d3d_y;
    obj->position.z = d3d_z;
    
    obj->rot_x = rx;
    obj->rot_y = ry;
    obj->rot_z = rz;
}

/* ===== Read material for an object ===== */
static void read_material(mw_reader_t *r, mw_object_t *obj) {
    obj->ambient[0]  = mw_read_f32(r);
    obj->ambient[1]  = mw_read_f32(r);
    obj->ambient[2]  = mw_read_f32(r);
    obj->ambient[3]  = mw_read_f32(r);
    
    obj->diffuse[0]  = mw_read_f32(r);
    obj->diffuse[1]  = mw_read_f32(r);
    obj->diffuse[2]  = mw_read_f32(r);
    obj->diffuse[3]  = mw_read_f32(r);
    
    obj->ambient[0]  = mw_read_f32(r); /* specular r */
    obj->ambient[1]  = mw_read_f32(r); /* specular g */
    obj->ambient[2]  = mw_read_f32(r); /* specular b */
    obj->ambient[3]  = mw_read_f32(r); /* specular a */
    
    /* emissive r,g,b,a */
    mw_read_f32(r); mw_read_f32(r); mw_read_f32(r); mw_read_f32(r);
    
    obj->size_param = mw_read_f32(r);  /* Power/shininess */
    
    int has_reflection = mw_read_u32(r);
    (void)has_reflection;
    
    int has_texture = mw_read_u32(r);
    if (has_texture) {
        mw_read_string(r, obj->texture, sizeof(obj->texture));
    }
}

/* ===== Main parse function (official format) ===== */
mw_level_t *meshworld_parse(const uint8_t *data, size_t size) {
    if (!data || size < 4) return NULL;
    
    mw_level_t *level = calloc(1, sizeof(mw_level_t));
    if (!level) return NULL;
    
    level->object_capacity = 128;
    level->objects = calloc(level->object_capacity, sizeof(mw_object_t));
    if (!level->objects) { free(level); return NULL; }
    
    /* Store full geometry data for later rendering */
    level->geometry_data = malloc(size);
    if (level->geometry_data) {
        memcpy(level->geometry_data, data, size);
        level->geometry_size = (int)size;
    }
    
    mw_reader_t r = { data, size, 0 };
    
    /* === SECTION 1: Ref Points === */
    uint32_t point_count = mw_read_u32(&r);
    for (uint32_t i = 0; i < point_count; i++) {
        char name[256];
        if (mw_read_string(&r, name, sizeof(name)) < 0) break;
        
        /* Position: exporter writes Max mPosition.x, mPosition.z, mPosition.y
         * In Max: Z is up, Y is forward. In D3D: Y is up, Z is forward.
         * So: D3D.x = Max.x, D3D.y = Max.z (up), D3D.z = Max.y (forward) */
        float max_x = mw_read_f32(&r);
        float max_z = mw_read_f32(&r);  /* Z-up in Max → Y-up in D3D */
        float max_y = mw_read_f32(&r);  /* Y-forward in Max → Z-forward in D3D */
        float d3d_x = max_x;
        float d3d_y = max_z;   /* Max Z-up becomes D3D Y-up */
        float d3d_z = max_y;   /* Max Y-forward becomes D3D Z-forward */
        
        /* Rotation: same x,z,y → x,y,z swap */
        float rx = mw_read_f32(&r);
        float rz = mw_read_f32(&r);
        float ry = mw_read_f32(&r);
        
        uint32_t has_material = mw_read_u32(&r);
        
        add_object(level, name, d3d_x, d3d_y, d3d_z, rx, ry, rz);
        mw_object_t *obj = &level->objects[level->object_count - 1];
        
        if (has_material) {
            read_material(&r, obj);
        }
    }
    
    /* === SECTION 2: Splines === */
    uint32_t spline_count = mw_read_u32(&r);
    for (uint32_t i = 0; i < spline_count; i++) {
        char name[256];
        if (mw_read_string(&r, name, sizeof(name)) < 0) break;
        
        uint32_t spline_pts = mw_read_u32(&r);
        for (uint32_t j = 0; j < spline_pts; j++) {
            mw_read_f32(&r); /* x */
            mw_read_f32(&r); /* z */
            mw_read_f32(&r); /* y */
        }
        
        /* If this is a BallPath, record it */
        if (strncmp(name, "BallPath", 8) == 0) {
            add_object(level, name, 0, 0, 0, 0, 0, 0);
        }
    }
    
    /* === SECTION 3: Lights === */
    uint32_t light_count = mw_read_u32(&r);
    for (uint32_t i = 0; i < light_count; i++) {
        uint32_t light_type = mw_read_u32(&r);
        if (light_type == 0) { /* DISTANTLIGHT */
            mw_read_f32(&r); mw_read_f32(&r); mw_read_f32(&r); /* pos x,z,y */
            mw_read_f32(&r); mw_read_f32(&r); mw_read_f32(&r); /* lookat x,z,y */
            mw_read_f32(&r); mw_read_f32(&r); mw_read_f32(&r); /* color r,g,b */
        }
    }
    
    /* === SECTION 4: Background & Ambient Colors === */
    level->bg_color.x = mw_read_f32(&r);
    level->bg_color.y = mw_read_f32(&r);
    level->bg_color.z = mw_read_f32(&r);
    
    level->ambient_color.x = mw_read_f32(&r);
    level->ambient_color.y = mw_read_f32(&r);
    level->ambient_color.z = mw_read_f32(&r);
    
    /* === SECTION 5: Global Vertex Buffer === */
    level->vertex_count = mw_read_u32(&r);
    if (level->vertex_count > 0 && level->vertex_count < 200000) {
        size_t vb_size = level->vertex_count * sizeof(mw_vertex_t);
        level->vertices = malloc(vb_size);
        if (level->vertices) {
            if (mw_read(&r, level->vertices, vb_size) < 0) {
                free(level->vertices);
                level->vertices = NULL;
                level->vertex_count = 0;
            }
        }
    } else {
        level->vertex_count = 0;
    }
    
    /* === SECTION 6: Octree Mesh Dump === */
    /* Read root cube bounds */
    if (r.pos + 24 <= r.size) {
        level->bounds_min.x = mw_read_f32(&r);
        level->bounds_min.y = mw_read_f32(&r);
        level->bounds_min.z = mw_read_f32(&r);
        level->bounds_max.x = mw_read_f32(&r);
        level->bounds_max.y = mw_read_f32(&r);
        level->bounds_max.z = mw_read_f32(&r);
    }
    
    /* Compute actual vertex bounding box (more reliable than octree cube) */
    if (level->vertices && level->vertex_count > 0) {
        float vmin[3] = {1e30f,1e30f,1e30f}, vmax[3] = {-1e30f,-1e30f,-1e30f};
        for (int i = 0; i < level->vertex_count; i++) {
            mw_vertex_t *v = &level->vertices[i];
            if (v->x < vmin[0]) vmin[0] = v->x;
            if (v->y < vmin[1]) vmin[1] = v->y;
            if (v->z < vmin[2]) vmin[2] = v->z;
            if (v->x > vmax[0]) vmax[0] = v->x;
            if (v->y > vmax[1]) vmax[1] = v->y;
            if (v->z > vmax[2]) vmax[2] = v->z;
        }
        /* Override octree bounds with vertex-computed bounds */
        level->bounds_min.x = vmin[0]; level->bounds_min.y = vmin[1]; level->bounds_min.z = vmin[2];
        level->bounds_max.x = vmax[0]; level->bounds_max.y = vmax[1]; level->bounds_max.z = vmax[2];
    }
    
    return level;
}

mw_level_t *meshworld_parse_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (sz <= 0) { fclose(f); return NULL; }
    
    uint8_t *data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    
    size_t read = fread(data, 1, (size_t)sz, f);
    fclose(f);
    
    mw_level_t *level = meshworld_parse(data, read);
    free(data);
    return level;
}

void meshworld_free(mw_level_t *level) {
    if (!level) return;
    if (level->objects) free(level->objects);
    if (level->vertices) free(level->vertices);
    if (level->geometry_data) free(level->geometry_data);
    free(level);
}