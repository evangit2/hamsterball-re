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

/* ===== Recursive octree walker for Section 6 ===== */
typedef struct {
    mw_reader_t *r;
    mw_level_t *level;
    int geom_cap;
    int total_indices;
} octree_ctx_t;

static void walk_octree(octree_ctx_t *ctx) {
    /* Read cube bounds (6 floats — skip them) */
    for (int i = 0; i < 6; i++) mw_read_f32(ctx->r);
    
    int submesh_count = (int)mw_read_u32(ctx->r);
    if (submesh_count > 0) {
        /* Internal node — recurse into each child */
        for (int i = 0; i < submesh_count; i++) {
            walk_octree(ctx);
        }
    } else {
        /* Leaf node — read geoms */
        int gc = (int)mw_read_u32(ctx->r);
        for (int g = 0; g < gc; g++) {
            if (ctx->level->geom_count >= ctx->geom_cap) {
                ctx->geom_cap *= 2;
                ctx->level->geoms = realloc(ctx->level->geoms, ctx->geom_cap * sizeof(mw_geom_t));
            }
            mw_geom_t *geom = &ctx->level->geoms[ctx->level->geom_count++];
            memset(geom, 0, sizeof(*geom));
            
            mw_read_string(ctx->r, geom->name, sizeof(geom->name));
            geom->ambient[0] = mw_read_f32(ctx->r); geom->ambient[1] = mw_read_f32(ctx->r);
            geom->ambient[2] = mw_read_f32(ctx->r); geom->ambient[3] = mw_read_f32(ctx->r);
            geom->diffuse[0] = mw_read_f32(ctx->r); geom->diffuse[1] = mw_read_f32(ctx->r);
            geom->diffuse[2] = mw_read_f32(ctx->r); geom->diffuse[3] = mw_read_f32(ctx->r);
            geom->specular[0] = mw_read_f32(ctx->r); geom->specular[1] = mw_read_f32(ctx->r);
            geom->specular[2] = mw_read_f32(ctx->r); geom->specular[3] = mw_read_f32(ctx->r);
            geom->emissive[0] = mw_read_f32(ctx->r); geom->emissive[1] = mw_read_f32(ctx->r);
            geom->emissive[2] = mw_read_f32(ctx->r); geom->emissive[3] = mw_read_f32(ctx->r);
            geom->power = mw_read_f32(ctx->r);
            int has_reflection = (int)mw_read_u32(ctx->r); (void)has_reflection;
            geom->has_texture = (int)mw_read_u32(ctx->r);
            if (geom->has_texture)
                mw_read_string(ctx->r, geom->texture, sizeof(geom->texture));
            
            geom->strip_count = (int)mw_read_u32(ctx->r);
            if (geom->strip_count > 0) {
                geom->strips = calloc(geom->strip_count, sizeof(mw_strip_t));
                for (int s = 0; s < geom->strip_count; s++) {
                    geom->strips[s].tri_count = (int)mw_read_u32(ctx->r);
                    geom->strips[s].vertex_offset = (int)mw_read_u32(ctx->r);
                    ctx->total_indices += geom->strips[s].tri_count * 3;
                }
            }
        }
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
    
    /* === Parse Section 6 octree: extract geoms and build triangle index buffer === */
    /* 
     * The octree format: Cube(6 floats), then:
     *   if subdivided: submesh_count > 0, then recursive children
     *   if leaf: submesh_count=0, then geom_count, then geoms
     * Each geom: name, materials, has_texture, texture, strip_count, 
     *   then per strip: (tri_count, vertex_ref_offset)
     */
    size_t sec6_start = r.pos;
    
    /* Fallback: if no geoms found, try simple flat parse (arena levels have no octree subdivision) */
    int geom_cap = 64;
    level->geoms = calloc(geom_cap, sizeof(mw_geom_t));
    level->geom_count = 0;
    int total_indices = 0;
    
    /* Simple approach for WarmUp/Arena levels: Section 6 is often just a single leaf
     * Root cube(6 floats) + 0 (no submeshes) + geom_count + geoms.
     * For levels with octree subdivision, we recurse. */
    r.pos = sec6_start;
    
    /* Read root cube */
    for (int i = 0; i < 6; i++) mw_read_f32(&r);
    
    int submesh_count = (int)mw_read_u32(&r);
    /* For now, handle the simple case: no submeshes (leaf node) */
    /* TODO: handle recursive octree for complex levels */
    
    if (submesh_count == 0) {
        /* Leaf node — read geoms directly */
        int gc = (int)mw_read_u32(&r);
        
        for (int g = 0; g < gc; g++) {
            if (level->geom_count >= geom_cap) {
                geom_cap *= 2;
                level->geoms = realloc(level->geoms, geom_cap * sizeof(mw_geom_t));
            }
            mw_geom_t *geom = &level->geoms[level->geom_count++];
            memset(geom, 0, sizeof(*geom));
            
            mw_read_string(&r, geom->name, sizeof(geom->name));
            geom->ambient[0] = mw_read_f32(&r); geom->ambient[1] = mw_read_f32(&r);
            geom->ambient[2] = mw_read_f32(&r); geom->ambient[3] = mw_read_f32(&r);
            geom->diffuse[0] = mw_read_f32(&r); geom->diffuse[1] = mw_read_f32(&r);
            geom->diffuse[2] = mw_read_f32(&r); geom->diffuse[3] = mw_read_f32(&r);
            geom->specular[0] = mw_read_f32(&r); geom->specular[1] = mw_read_f32(&r);
            geom->specular[2] = mw_read_f32(&r); geom->specular[3] = mw_read_f32(&r);
            geom->emissive[0] = mw_read_f32(&r); geom->emissive[1] = mw_read_f32(&r);
            geom->emissive[2] = mw_read_f32(&r); geom->emissive[3] = mw_read_f32(&r);
            geom->power = mw_read_f32(&r);
            int has_reflection = (int)mw_read_u32(&r); (void)has_reflection;
            geom->has_texture = (int)mw_read_u32(&r);
            if (geom->has_texture)
                mw_read_string(&r, geom->texture, sizeof(geom->texture));
            
            geom->strip_count = (int)mw_read_u32(&r);
            if (geom->strip_count > 0) {
                geom->strips = calloc(geom->strip_count, sizeof(mw_strip_t));
                for (int s = 0; s < geom->strip_count; s++) {
                    geom->strips[s].tri_count = (int)mw_read_u32(&r);
                    geom->strips[s].vertex_offset = (int)mw_read_u32(&r);
                    total_indices += geom->strips[s].tri_count * 3;
                }
            }
        }
    }
    
    /* Build the flat triangle-list index buffer from strips */
    if (total_indices > 0 && level->geoms) {
        level->indices = calloc(total_indices, sizeof(uint32_t));
        level->index_count = 0;
        
        for (int g = 0; g < level->geom_count; g++) {
            mw_geom_t *geom = &level->geoms[g];
            for (int s = 0; s < geom->strip_count; s++) {
                mw_strip_t *strip = &geom->strips[s];
                int ntri = strip->tri_count;
                int base = strip->vertex_offset;
                
                /* Convert triangle strip to triangle list
                 * Strip: vertices base+0..base+ntri+1
                 * For each triangle t: even=(t,t+1,t+2), odd=(t,t+2,t+1)
                 */
                for (int t = 0; t < ntri && level->index_count + 3 <= total_indices; t++) {
                    if (t % 2 == 0) {
                        level->indices[level->index_count++] = base + t;
                        level->indices[level->index_count++] = base + t + 1;
                        level->indices[level->index_count++] = base + t + 2;
                    } else {
                        level->indices[level->index_count++] = base + t;
                        level->indices[level->index_count++] = base + t + 2;
                        level->indices[level->index_count++] = base + t + 1;
                    }
                }
            }
        }
        
        printf("[MeshWorld] Built %d triangle indices from %d geoms\n",
               level->index_count, level->geom_count);
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
    if (level->indices) free(level->indices);
    if (level->geoms) {
        for (int g = 0; g < level->geom_count; g++) {
            if (level->geoms[g].strips) free(level->geoms[g].strips);
        }
        free(level->geoms);
    }
    free(level);
}