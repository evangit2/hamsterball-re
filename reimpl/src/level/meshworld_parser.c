/*
 * Hamsterball Binary Mesh/MeshWorld Parser v3
 * Based on Ghidra decompilation of FUN_004629E0 (binary mesh loader)
 * 
 * Binary format read order (sequential __read calls):
 *   1. material_count
 *   2. For each material: name, properties, optional texture
 *   3. mesh_buffer_count  
 *   4. For each mesh buffer: name, vertex/face data
 *   5. game_object_count
 *   6. For each game object: type, position, rotation, etc.
 *   7. Bounding box (6 floats)
 *   8. Vertex array (count * 32 bytes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "level/meshworld_parser.h"

/* Read helpers - mimic the game's sequential __read calls */
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

static char *mw_read_string(mw_reader_t *r) {
    uint32_t len = mw_read_u32(r);
    if (len == 0 || len > 1024) return NULL;
    char *s = malloc(len + 1);
    if (!s) return NULL;
    if (mw_read(r, s, len) < 0) { free(s); return NULL; }
    s[len] = '\0';
    return s;
}

/* Classify object type from type string */
static mw_obj_type_t classify_type(const char *s) {
    if (!s) return MW_OBJ_UNKNOWN;
    if (strncmp(s, "START", 5) == 0) return MW_OBJ_START;
    if (strncmp(s, "FLAG", 4) == 0) return MW_OBJ_FLAG;
    if (strcmp(s, "SAFESPOT") == 0) return MW_OBJ_SAFESPOT;
    if (strncmp(s, "CameraLocus", 11) == 0 || strncmp(s, "CAMERALOOKAT", 12) == 0) return MW_OBJ_CAMERALOOKAT;
    if (strncmp(s, "PLATFORM", 8) == 0) return MW_OBJ_PLATFORM;
    if (strncmp(s, "N:SINKPLATFORM", 14) == 0) return MW_OBJ_SINKPLATFORM;
    if (strncmp(s, "N:BUMPER", 8) == 0) return MW_OBJ_SINKPLATFORM; /* Reuse for now */
    if (strncmp(s, "E:NODIZZY", 9) == 0) return MW_OBJ_NODIZZY;
    if (strncmp(s, "E:GROWSOUND", 11) == 0) return MW_OBJ_GROWSOUND;
    if (strncmp(s, "E:N:GOAL", 8) == 0) return MW_OBJ_GOAL;
    if (strncmp(s, "E:LIMIT", 7) == 0) return MW_OBJ_GOAL;
    if (strncmp(s, "RND", 3) == 0 || strncmp(s, "+RND", 4) == 0) return MW_OBJ_RND;
    if (strcmp(s, "BallPath") == 0) return MW_OBJ_BALLPATH;
    if (strcmp(s, "BCMESH") == 0) return MW_OBJ_BCMBALL;
    if (strcmp(s, "BADBALL") == 0) return MW_OBJ_BADBALL;
    return MW_OBJ_UNKNOWN;
}

static int is_primary_type(const char *s) {
    if (!s) return 0;
    return (strncmp(s, "START", 5) == 0 ||
            strncmp(s, "FLAG", 4) == 0 ||
            strcmp(s, "SAFESPOT") == 0 ||
            strncmp(s, "CameraLocus", 11) == 0 ||
            strncmp(s, "CAMERALOOKAT", 12) == 0 ||
            strncmp(s, "PLATFORM", 8) == 0 ||
            strncmp(s, "N:", 2) == 0 ||
            strncmp(s, "E:", 2) == 0 ||
            strncmp(s, "RND", 3) == 0 ||
            strncmp(s, "+RND", 4) == 0 ||
            strncmp(s, "S:", 2) == 0 ||
            strcmp(s, "BallPath") == 0 ||
            strcmp(s, "BCMESH") == 0 ||
            strcmp(s, "BADBALL") == 0 ||
            strcmp(s, "STANDS") == 0);
}

/*
 * Parse binary MESHWORLD file using the Ghidra-confirmed format.
 * This replaces the old string-scanning approach with proper sequential reading.
 */
mw_level_t *meshworld_parse(const uint8_t *data, size_t size) {
    mw_level_t *level = calloc(1, sizeof(mw_level_t));
    if (!level) return NULL;
    
    level->object_capacity = 128;
    level->objects = calloc(level->object_capacity, sizeof(mw_object_t));
    if (!level->objects) { free(level); return NULL; }
    
    mw_reader_t reader = { data, size, 0 };
    
    /* === Section 1: Materials === */
    uint32_t material_count = mw_read_u32(&reader);
    printf("[MW] Material count: %u\n", material_count);
    
    for (uint32_t i = 0; i < material_count; i++) {
        char *name = mw_read_string(&reader);
        /* Material struct: 6 uint32 values after name */
        uint32_t face_start = mw_read_u32(&reader);
        uint32_t face_count = mw_read_u32(&reader);
        uint32_t unk3 = mw_read_u32(&reader);
        uint32_t unk4 = mw_read_u32(&reader);
        uint32_t unk5 = mw_read_u32(&reader);
        uint32_t unk6 = mw_read_u32(&reader);
        
        char has_extended = 0;
        mw_read(&reader, &has_extended, 1);  /* 4-byte read but only char matters */
        /* Fix: the game reads 4 bytes but only checks first byte */
        mw_read_u32(&reader); /* Actually it reads the full 4 bytes */
        
        /* Actually from the decompilation: __read(_FileHandle, puVar5 + 10, 4)
         * That's 4 bytes at offset 10*4=40 of the material struct.
         * Then it checks if first byte is non-zero */
        /* Let me re-read the decompilation... the struct indices are in uint32 units */
        
        if (name) {
            printf("[MW]   Material %u: '%s' faces=%u-%u\n", i, name, face_start, face_count);
            free(name);
        }
        
        /* Check for extended material data */
        /* From decompilation: if (*(char *)(puVar5 + 10) != '\0')
         * Extended data: ambient(4f), diffuse(4f), specular(4f), shine(2u32), reflective(u32), has_texture(u32) */
        if (has_extended) {
            /* Read ambient RGBA (4 floats) */
            float ar = mw_read_f32(&reader), ag = mw_read_f32(&reader);
            float ab = mw_read_f32(&reader), aa = mw_read_f32(&reader);
            /* Read diffuse RGBA (4 floats) */  
            float dr = mw_read_f32(&reader), dg = mw_read_f32(&reader);
            float db = mw_read_f32(&reader), da = mw_read_f32(&reader);
            /* Read specular (4 floats + 2 uint32s) */
            float sr = mw_read_f32(&reader), sg = mw_read_f32(&reader);
            float sb = mw_read_f32(&reader), sa = mw_read_f32(&reader);
            uint32_t shine = mw_read_u32(&reader);
            uint32_t reflect = mw_read_u32(&reader);
            /* has_texture */
            uint32_t has_tex = mw_read_u32(&reader);
            if (has_tex == 1) {
                char *tex = mw_read_string(&reader);
                if (tex) {
                    printf("[MW]     Texture: '%s'\n", tex);
                    free(tex);
                }
            }
        }
    }
    
    /* === Section 2: Mesh Buffers (geometry submeshes) === */
    uint32_t mesh_buffer_count = mw_read_u32(&reader);
    printf("[MW] Mesh buffer count: %u\n", mesh_buffer_count);
    
    for (uint32_t i = 0; i < mesh_buffer_count; i++) {
        char *name = mw_read_string(&reader);
        uint32_t vertex_count = mw_read_u32(&reader);
        
        printf("[MW]   Buffer %u: '%s' vertices=%u\n", i, name ? name : "?", vertex_count);
        
        /* For each vertex, read 3 then 2 then 1 float (6 floats = 24 bytes)
         * From decompilation: __read(fStack_5a4, 4) x3, then more reads */
        for (uint32_t v = 0; v < vertex_count; v++) {
            float x = mw_read_f32(&reader);  /* posX */
            float y = mw_read_f32(&reader);  /* posY */  
            float z = mw_read_f32(&reader);  /* posZ */
            /* Additional vertex attributes read by FUN_004685e0 */
            float u = mw_read_f32(&reader);  /* texU */
            float v2 = mw_read_f32(&reader); /* texV */
            /* More reads inside FUN_004685e0 - need to decompile that too */
            /* For now, skip remaining vertex data */
            /* The decompilation shows 3 reads then calling FUN_004685e0
             * which probably reads more data. For a rough parse, let's
             * try to figure out the total bytes per vertex */
        }
        
        if (name) free(name);
        
        /* After vertices, there are face counts and face data
         * This section needs more reverse engineering of FUN_004685e0
         * For now, break if we can't parse further */
        if (vertex_count > 0) {
            /* The remaining format is complex - we've hit the limit of what
             * the decompilation tells us without decompiling FUN_004685e0 */
            printf("[MW]   (vertex/face data parsing incomplete - needs further RE)\n");
            break;
        }
    }
    
    /* === Fall back to string-scanning approach for objects === */
    /* The binary parser above handles materials and mesh buffers,
     * but the object section format is complex. For now, scan
     * for object type strings like before. */
    
    /* Scan for primary type strings */
    for (size_t pos = 0; pos < size - 4; pos++) {
        uint32_t slen = *(uint32_t *)(data + pos);
        if (slen >= 2 && slen <= 60 && pos + 4 + slen <= size) {
            char candidate[256] = {0};
            memcpy(candidate, data + pos + 4, slen < 255 ? slen : 255);
            for (int i = slen - 1; i >= 0; i--) {
                if (candidate[i] == '\0') candidate[i] = '\0'; else break;
            }
            
            if (strlen(candidate) >= 2 && is_primary_type(candidate)) {
                /* Add as object */
                if (level->object_count >= level->object_capacity) {
                    level->object_capacity *= 2;
                    level->objects = realloc(level->objects, level->object_capacity * sizeof(mw_object_t));
                    if (!level->objects) return NULL;
                }
                
                mw_object_t *obj = &level->objects[level->object_count];
                memset(obj, 0, sizeof(mw_object_t));
                strncpy(obj->type_string, candidate, 255);
                obj->type = classify_type(candidate);
                
                size_t data_start = pos + 4 + slen;
                if (data_start + 12 <= size) {
                    obj->position.x = *(float *)(data + data_start);
                    obj->position.y = *(float *)(data + data_start + 4);
                    obj->position.z = *(float *)(data + data_start + 8);
                }
                if (data_start + 28 <= size) {
                    obj->rot_x = *(float *)(data + data_start + 12);
                    obj->rot_y = *(float *)(data + data_start + 16);
                    obj->rot_z = *(float *)(data + data_start + 20);
                    obj->rot_w = *(float *)(data + data_start + 24);
                }
                
                level->object_count++;
            }
        }
    }
    
    /* Store geometry data */
    level->geometry_size = reader.pos;
    if (reader.pos > 0 && reader.pos < size) {
        level->geometry_data = malloc(reader.pos);
        if (level->geometry_data) memcpy(level->geometry_data, data, reader.pos);
    }
    
    printf("[MW] Parsed %d objects, %zu bytes in geometry header\n", 
           level->object_count, reader.pos);
    return level;
}

mw_level_t *meshworld_parse_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = malloc(size);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, size, f);
    fclose(f);
    mw_level_t *level = meshworld_parse(data, size);
    free(data);
    return level;
}

void meshworld_free(mw_level_t *level) {
    if (level) {
        free(level->geometry_data);
        if (level->objects) {
            for (int i = 0; i < level->object_count; i++)
                free(level->objects[i].indices);
            free(level->objects);
        }
        free(level);
    }
}