/*
 * Hamsterball Binary MeshWorld Parser v7
 * 
 * Binary MESHWORLD format per Ghidra decompilation at 0x4629e0:
 *   1. u32 material_count — then per material: name_len(u32)+name(len)+6×u32+ext_flag(4B)+optional color/tex
 *   2. u32 meshbuf_count — then per buf: data_len(u32)+data(len)+face_count(u32)+face_data
 *   3. u32 obj_count — then per obj: type(i32)+type-dependent data (pos/rot/scale etc)
 *   4. bbox: 6 × float (min_xyz then max_xyz)
 *   5. u32 vertex_count — then vertex_count × 32 bytes of vertex data
 *
 * SpawnPlatform has 0/0/0 counts so sections 1-3 are empty.
 * Complex levels (WarmUp) have items in section 1 that include both materials and objects.
 *
 * Vertex format (32 bytes each):
 *   position(3×float) + normal(3×float) + uv(2×float)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "level/meshworld_parser.h"

/* ===== Binary reader helpers ===== */
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

/* Read a length-prefixed string: u32 len + len bytes (may include NUL) */
static char *mw_read_lps(mw_reader_t *r) {
    uint32_t len = mw_read_u32(r);
    if (len == 0 || len > 4096) return NULL;
    if (r->pos + len > r->size) return NULL;
    char *s = (char *)malloc(len + 1);
    if (!s) return NULL;
    memcpy(s, r->data + r->pos, len);
    s[len] = '\0';
    r->pos += len;
    return s;
}

/* ===== Object type classification ===== */
static mw_obj_type_t classify_type(const char *s) {
    if (!s) return MW_OBJ_UNKNOWN;
    if (strncmp(s, "START", 5) == 0) return MW_OBJ_START;
    if (strncmp(s, "FLAG", 4) == 0) return MW_OBJ_FLAG;
    if (strcmp(s, "SAFESPOT") == 0) return MW_OBJ_SAFESPOT;
    if (strncmp(s, "CAMERALOOKAT", 12) == 0) return MW_OBJ_CAMERALOOKAT;
    if (strncmp(s, "PLATFORM", 8) == 0) return MW_OBJ_PLATFORM;
    if (strncmp(s, "N:SINKPLATFORM", 14) == 0) return MW_OBJ_SINKPLATFORM;
    if (strncmp(s, "N:", 2) == 0 || strncmp(s, "E:", 2) == 0 || strncmp(s, "S:", 2) == 0) return MW_OBJ_GOAL;
    if (strncmp(s, "RND", 3) == 0 || strncmp(s, "+RND", 4) == 0) return MW_OBJ_RND;
    if (strcmp(s, "STANDS") == 0) return MW_OBJ_STANDS;
    if (strcmp(s, "BADBALL") == 0) return MW_OBJ_BADBALL;
    if (strcmp(s, "BCMESH") == 0) return MW_OBJ_BCMBALL;
    if (strcmp(s, "BallPath") == 0) return MW_OBJ_BALLPATH;
    return MW_OBJ_UNKNOWN;
}

/* ===== Parse section 1: Materials (0x7C structure each) =====
 * Per Ghidra decomp: alloc 0x7C, init vtable, read name_len + name,
 * read 6 × u32 (puVar5[1..6]), read ext_flag (puVar5+10, check low byte),
 * if ext: read ambient(4f), diffuse(4f), specular(4f), shine(f),
 *         reflective(u32), has_tex(u32), if has_tex==1: texname_len + texname
 */
static int parse_materials(mw_reader_t *r, mw_level_t *level, uint32_t count) {
    level->material_count = count;
    if (count == 0 || count > 10000) return 0;
    
    level->materials = (mw_material_t *)calloc(count, sizeof(mw_material_t));
    if (!level->materials) return -1;
    
    for (uint32_t i = 0; i < count; i++) {
        if (r->pos + 8 > r->size) break;
        
        mw_material_t *mat = &level->materials[i];
        memset(mat, 0, sizeof(*mat));
        
        /* Read name: u32 len + len bytes */
        char *name = mw_read_lps(r);
        if (!name) break;
        strncpy(mat->name, name, sizeof(mat->name) - 1);
        free(name);
        
        /* 6 × u32 fields (face_start, face_count, etc.) */
        if (r->pos + 24 > r->size) break;
        mat->face_start = mw_read_u32(r);
        mat->face_count = mw_read_u32(r);
        mat->unk3 = mw_read_u32(r);
        mat->unk4 = mw_read_u32(r);
        mat->unk5 = mw_read_u32(r);
        mat->unk6 = mw_read_u32(r);
        
        /* Extended flag (u32, check low byte) */
        if (r->pos + 4 > r->size) break;
        uint32_t ext_raw = mw_read_u32(r);
        uint8_t ext_flag = (uint8_t)(ext_raw & 0xFF);
        
        if (ext_flag != 0 && r->pos + 76 <= r->size) {
            /* Ambient RGBA (4 floats) — stored at puVar5[0x10..0x13] */
            for (int j = 0; j < 4; j++) mat->ambient[j] = mw_read_f32(r);
            /* Diffuse RGBA (4 floats) — stored at puVar5[0x0c..0x0f] */
            for (int j = 0; j < 4; j++) mat->diffuse[j] = mw_read_f32(r);
            /* Specular RGBA (4 floats) — stored at puVar5[0x14..0x17] */
            for (int j = 0; j < 4; j++) mat->specular[j] = mw_read_f32(r);
            /* Emission RGBA (4 floats) — stored at puVar5[0x18..0x1b] */
            float emission[4];
            for (int j = 0; j < 4; j++) emission[j] = mw_read_f32(r);
            (void)emission; /* not stored in our simplified struct yet */
            /* Shininess — puVar5[0x1c] */
            mat->shine = mw_read_f32(r);
            /* Reflective flag — bool at puVar5+0x79 */
            mat->has_reflective = mw_read_u32(r) != 0;
            /* Has texture — checked with == 1 */
            uint32_t has_tex = mw_read_u32(r);
            mat->has_texture = (uint8_t)(has_tex & 0xFF);
            
            if (mat->has_texture == 1 && r->pos + 4 <= r->size) {
                char *tex = mw_read_lps(r);
                if (tex) {
                    strncpy(mat->texture, tex, sizeof(mat->texture) - 1);
                    free(tex);
                }
            }
        }
    }
    return 0;
}

/* ===== Parse section 2: Mesh Buffers (0x54 structure each) =====
 * Per decomp: alloc 0x54, read data_len(u32), alloc data, read data,
 * read face_count(u32), per face: read 3 × u32, call FUN_004685e0
 */
static int parse_meshbufs(mw_reader_t *r, mw_level_t *level, uint32_t count) {
    level->mesh_buffer_count = count;
    if (count == 0 || count > 10000) return 0;
    
    level->mesh_buffers = (mw_mesh_buffer_t *)calloc(count, sizeof(mw_mesh_buffer_t));
    if (!level->mesh_buffers) return -1;
    
    for (uint32_t i = 0; i < count; i++) {
        if (r->pos + 8 > r->size) break;
        
        mw_mesh_buffer_t *buf = &level->mesh_buffers[i];
        memset(buf, 0, sizeof(*buf));
        
        /* Data length + raw data */
        uint32_t data_len = mw_read_u32(r);
        if (data_len > 0 && data_len < 1000000 && r->pos + data_len <= r->size) {
            /* Skip raw mesh data for now */
            r->pos += data_len;
        }
        
        /* Face count + skip face data (3 × 4 bytes per face) */
        if (r->pos + 4 > r->size) break;
        buf->face_count = mw_read_u32(r);
        if (buf->face_count > 0 && buf->face_count < 1000000) {
            uint32_t face_bytes = buf->face_count * 12;  /* 3 reads of 4 bytes */
            if (r->pos + face_bytes <= r->size) {
                r->pos += face_bytes;
            }
        }
    }
    return 0;
}

/* ===== Parse section 3: Objects (0xD4 structure each) =====
 * Per Ghidra decomp at 0x4629e0 lines 210-258:
 *   read(obj_count, 4)
 *   for each: read(type, 4) → if type==0: set internal type=3,
 *     read 3 floats → SetPosition (vtable+4)
 *     read 3 floats → SetRotation (vtable+8) 
 *     read 3 floats → SetScale (FUN_00453180 quaternion builder)
 *     Then set internal defaults (no more file reads for this object).
 *   If type!=0: no reads — just increment counter.
 *
 * NOTE: Arena levels always have obj_count=1 with type=0.
 * The 9 floats are constant-ish per level (pos+rot+scale for a default object).
 * After objects, the decomp reads bbox(6f) then vertex_count then vertex data.
 */
static int parse_objects(mw_reader_t *r, mw_level_t *level, uint32_t count) {
    level->object_count = 0;
    level->object_capacity = 0;
    level->objects = NULL;
    
    for (uint32_t i = 0; i < count; i++) {
        if (r->pos + 4 > r->size) break;
        
        int32_t obj_type = (int32_t)mw_read_u32(r);
        
        if (obj_type == 0) {
            /* Type 0 (START): reads position(3f) + rotation(3f) + scale(3f) = 9 floats */
            if (r->pos + 36 > r->size) break;
            
            /* Allocate object if we have room */
            if (level->object_count >= level->object_capacity) {
                int new_cap = level->object_capacity ? level->object_capacity * 2 : 4;
                mw_object_t *new_objs = (mw_object_t *)realloc(level->objects, new_cap * sizeof(mw_object_t));
                if (!new_objs) break;
                level->objects = new_objs;
                level->object_capacity = new_cap;
            }
            
            mw_object_t *obj = &level->objects[level->object_count];
            memset(obj, 0, sizeof(*obj));
            obj->type = MW_OBJ_START;  /* type 0 → START (internal type 3) */
            
            /* Position */
            obj->position.x = mw_read_f32(r);
            obj->position.y = mw_read_f32(r);
            obj->position.z = mw_read_f32(r);
            
            /* Rotation (euler angles) */
            obj->rot_x = mw_read_f32(r);
            obj->rot_y = mw_read_f32(r);
            obj->rot_z = mw_read_f32(r);
            
            /* Scale */
            obj->transform[0] = mw_read_f32(r);
            obj->transform[1] = mw_read_f32(r);
            obj->transform[2] = mw_read_f32(r);
            
            level->object_count++;
        }
        /* If type != 0: no file reads — just skip */
    }
    
    printf("[MW] Parsed %d objects\n", level->object_count);
    return 0;
}

/* ===== Find bbox+vertices by scanning from end of file =====
 * The last thing in the file is: bbox(6 x float) + vertex_count(u32) + vertex_data
 * We can find it by looking backwards: vertex_data is count*32 bytes,
 * then vertex_count, then 6 floats that should be reasonable bounding box values.
 */
static int find_bbox_and_vertices(const uint8_t *data, size_t size,
                                    float *bbox, uint32_t *vtx_count, size_t *vtx_data_offset) {
    /* Try different vertex counts, working backwards from end */
    for (int try_count = 1; try_count < 100000; try_count++) {
        /* vertex_data = try_count * 32 bytes at end
         * before that: vertex_count (4 bytes)
         * before that: bbox (24 bytes = 6 floats)
         */
        size_t needed = try_count * 32 + 4 + 24;
        if (needed > size) break;
        
        size_t bbox_off = size - needed;
        
        /* Read candidate bbox */
        float bb[6];
        for (int i = 0; i < 6; i++) {
            bb[i] = *(float *)(data + bbox_off + i * 4);
        }
        
        /* Validate: bbox values should be in reasonable game-world range (-5000 to 5000) */
        int valid = 1;
        for (int i = 0; i < 6; i++) {
            if (bb[i] < -5000.0f || bb[i] > 5000.0f || bb[i] != bb[i]) { /* NaN check */
                valid = 0;
                break;
            }
        }
        if (!valid) continue;
        
        /* min should be < max */
        if (bb[0] >= bb[3] || bb[1] >= bb[4] || bb[2] >= bb[5]) continue;
        
        /* Check vertex_count */
        uint32_t vc = *(uint32_t *)(data + bbox_off + 24);
        if (vc != (uint32_t)try_count) continue;
        
        /* Validate first vertex — should be within bbox-ish range */
        if (try_count > 0) {
            float x = *(float *)(data + bbox_off + 28);
            float y = *(float *)(data + bbox_off + 32);
            float z = *(float *)(data + bbox_off + 36);
            /* Vertices should be within a reasonable range of the bbox */
            if (x < -10000 || x > 10000 || y < -10000 || y > 10000 || z < -10000 || z > 10000)
                continue;
        }
        
        /* Found it! */
        for (int i = 0; i < 6; i++) bbox[i] = bb[i];
        *vtx_count = vc;
        *vtx_data_offset = bbox_off + 28;  /* after bbox + vertex_count */
        return 1;
    }
    return 0;
}

/* ===== Main parser — matches Ghidra decompilation flow ===== */
mw_level_t *meshworld_parse(const uint8_t *data, size_t size) {
    if (!data || size < 12) return NULL;
    
    mw_reader_t r = { data, size, 0 };
    
    mw_level_t *level = (mw_level_t *)calloc(1, sizeof(mw_level_t));
    if (!level) return NULL;
    
    /* ===== Section 1: Material count + materials ===== */
    uint32_t mat_count = mw_read_u32(&r);
    printf("[MW] material_count = %u\n", mat_count);
    if (mat_count < 10000) {
        parse_materials(&r, level, mat_count);
    }
    
    /* ===== Section 2: Mesh buffer count + buffers ===== */
    if (r.pos + 4 > r.size) goto try_scan;
    uint32_t buf_count = mw_read_u32(&r);
    printf("[MW] meshbuf_count = %u\n", buf_count);
    if (buf_count < 10000) {
        parse_meshbufs(&r, level, buf_count);
    }
    
    /* ===== Section 3: Object count + objects ===== */
    if (r.pos + 4 > r.size) goto try_scan;
    uint32_t obj_count = mw_read_u32(&r);
    printf("[MW] obj_count = %u\n", obj_count);
    if (obj_count < 10000) {
        parse_objects(&r, level, obj_count);
    }
    
    /* ===== Section 4: Bounding Box (6 floats) =====
     * Per Ghidra decomp lines 260-265: reads 6 × 4 bytes into Level struct.
     * NOTE: For many Arena levels, the bbox values in the file are INVALID
     * (min > max for some axes) — they appear to be placeholder/garbage.
     * We read them but will recompute from vertex data below if invalid.
     */
    if (r.pos + 24 <= r.size) {
        level->bbox_min_x = mw_read_f32(&r);
        level->bbox_min_y = mw_read_f32(&r);
        level->bbox_min_z = mw_read_f32(&r);
        level->bbox_max_x = mw_read_f32(&r);
        level->bbox_max_y = mw_read_f32(&r);
        level->bbox_max_z = mw_read_f32(&r);
    }
    
    /* ===== Section 5: Vertex Count + Vertex Data =====
     * Per Ghidra decomp lines 266-270: read vertex_count(u32), 
     * allocate count*32 bytes, read vertex data.
     * Vertex format: position(3f) + normal(3f) + uv(2f) = 32 bytes.
     */
    if (r.pos + 4 <= r.size) {
        level->vertex_count = mw_read_u32(&r);
    }
    
    if (level->vertex_count > 0 && level->vertex_count < 1000000) {
        level->vertices = (mw_vertex_t *)calloc(level->vertex_count, sizeof(mw_vertex_t));
        if (level->vertices) {
            for (uint32_t i = 0; i < level->vertex_count && r.pos + 32 <= r.size; i++) {
                mw_vertex_t *v = &level->vertices[i];
                v->x  = mw_read_f32(&r);
                v->y  = mw_read_f32(&r);
                v->z  = mw_read_f32(&r);
                v->nx = mw_read_f32(&r);
                v->ny = mw_read_f32(&r);
                v->nz = mw_read_f32(&r);
                v->u  = mw_read_f32(&r);
                v->v  = mw_read_f32(&r);
            }
        }
    }
    
    /* ===== Validate/Recompute BBox from vertex data =====
     * Many level files have placeholder bbox values that are invalid.
     * Recompute if min > max OR if bbox is all-zeros (degenerate).
     */
    if (level->vertex_count > 0 && level->vertices) {
        int bbox_invalid = (level->bbox_min_x > level->bbox_max_x) ||
                           (level->bbox_min_y > level->bbox_max_y) ||
                           (level->bbox_min_z > level->bbox_max_z) ||
                           (level->bbox_min_x == 0 && level->bbox_max_x == 0 &&
                            level->bbox_min_y == 0 && level->bbox_max_y == 0 &&
                            level->bbox_min_z == 0 && level->bbox_max_z == 0);
        if (bbox_invalid) {
            level->bbox_min_x = level->bbox_max_x = level->vertices[0].x;
            level->bbox_min_y = level->bbox_max_y = level->vertices[0].y;
            level->bbox_min_z = level->bbox_max_z = level->vertices[0].z;
            for (int i = 1; i < level->vertex_count; i++) {
                mw_vertex_t *v = &level->vertices[i];
                if (v->x < level->bbox_min_x) level->bbox_min_x = v->x;
                if (v->y < level->bbox_min_y) level->bbox_min_y = v->y;
                if (v->z < level->bbox_min_z) level->bbox_min_z = v->z;
                if (v->x > level->bbox_max_x) level->bbox_max_x = v->x;
                if (v->y > level->bbox_max_y) level->bbox_max_y = v->y;
                if (v->z > level->bbox_max_z) level->bbox_max_z = v->z;
            }
            printf("[MW] Bbox was invalid — recomputed from vertices\n");
        }
    }
    
    printf("[MW] Parsed %d vertices, %d objects, %d materials, bbox (%.0f,%.0f,%.0f)-(%.0f,%.0f,%.0f)\n",
           level->vertex_count, level->object_count, level->material_count,
           level->bbox_min_x, level->bbox_min_y, level->bbox_min_z,
           level->bbox_max_x, level->bbox_max_y, level->bbox_max_z);
    return level;
    
    /* ===== Fallback: scan from end for bbox+vertices (UNUSED — kept for safety) ===== */
try_scan:
    printf("[MW] WARNING: Fell through to scan fallback — sequential parse incomplete\n");
    return level;
}

mw_level_t *meshworld_parse_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (sz <= 0) { fclose(f); return NULL; }
    
    uint8_t *data = (uint8_t *)malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    
    size_t read_count = fread(data, 1, (size_t)sz, f);
    fclose(f);
    
    mw_level_t *level = meshworld_parse(data, read_count);
    free(data);
    return level;
}

void meshworld_free(mw_level_t *level) {
    if (!level) return;
    if (level->objects) free(level->objects);
    if (level->materials) free(level->materials);
    if (level->mesh_buffers) free(level->mesh_buffers);
    if (level->vertices) free(level->vertices);
    if (level->geometry_data) free(level->geometry_data);
    free(level);
}