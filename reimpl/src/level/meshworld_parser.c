/*
 * Hamsterball Binary MeshWorld Parser v4
 * Based on Ghidra decompilation of 0x4629E0 (binary loader)
 * 
 * The binary format is complex with section-dependent parsing.
 * For reliable object extraction, we use string-scanning which
 * matches how the game's own text-format parser works.
 * 
 * Binary sections: materials → mesh buffers → game objects → bbox → vertices
 * We scan for game objects by finding length-prefixed type strings.
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

/* ===== String type classification ===== */
static mw_obj_type_t classify_type(const char *s) {
    if (!s) return MW_OBJ_UNKNOWN;
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

static int is_primary_type(const char *s) {
    if (!s || strlen(s) < 2) return 0;
    return (strncmp(s, "START", 5) == 0 ||
            strncmp(s, "FLAG", 4) == 0 ||
            strcmp(s, "SAFESPOT") == 0 ||
            strncmp(s, "CameraLocus", 11) == 0 ||
            strncmp(s, "CAMERALOOKAT", 12) == 0 ||
            strncmp(s, "PLATFORM", 8) == 0 ||
            strncmp(s, "N:", 2) == 0 ||
            strncmp(s, "E:", 2) == 0 ||
            strncmp(s, "S:", 2) == 0 ||
            strncmp(s, "RND", 3) == 0 ||
            strncmp(s, "+RND", 4) == 0 ||
            strcmp(s, "STANDS") == 0 ||
            strcmp(s, "BADBALL") == 0 ||
            strcmp(s, "BCMESH") == 0 ||
            strcmp(s, "BallPath") == 0);
}

/* ===== Object addition ===== */
static void add_object(mw_level_t *level, const char *type_str,
                       float x, float y, float z,
                       const uint8_t *data, size_t data_size, size_t obj_start) {
    if (level->object_count >= level->object_capacity) {
        level->object_capacity *= 2;
        level->objects = realloc(level->objects, level->object_capacity * sizeof(mw_object_t));
    }
    
    mw_object_t *obj = &level->objects[level->object_count++];
    memset(obj, 0, sizeof(*obj));
    
    obj->type = classify_type(type_str);
    strncpy(obj->type_string, type_str, sizeof(obj->type_string) - 1);
    obj->position.x = x;
    obj->position.y = y;
    obj->position.z = z;
    
    /* Try to read additional fields after position */
    if (obj_start + 12 + 16 <= data_size) {
        const float *extra = (const float *)(data + obj_start + 12);
        obj->rot_x = extra[0]; obj->rot_y = extra[1];
        obj->rot_z = extra[2]; obj->rot_w = extra[3];
    }
    if (obj_start + 12 + 16 + 32 <= data_size) {
        const float *mat = (const float *)(data + obj_start + 12 + 16);
        for (int i = 0; i < 8; i++) obj->transform[i] = mat[i];
        for (int i = 0; i < 4; i++) obj->diffuse[i] = mat[8+i];
        for (int i = 0; i < 4; i++) obj->ambient[i] = mat[12+i];
        obj->size_param = mat[16];
    }
}

/* ===== Main parse function ===== */
mw_level_t *meshworld_parse(const uint8_t *data, size_t size) {
    if (!data || size < 4) return NULL;
    
    mw_level_t *level = calloc(1, sizeof(mw_level_t));
    if (!level) return NULL;
    
    level->object_capacity = 128;
    level->objects = calloc(level->object_capacity, sizeof(mw_object_t));
    if (!level->objects) { free(level); return NULL; }
    
    /* Store geometry reference for later (rendering needs it) */
    level->geometry_data = malloc(size);
    if (level->geometry_data) {
        memcpy(level->geometry_data, data, size);
        level->geometry_size = (int)size;
    }
    
    /* Scan for length-prefixed strings that identify game objects.
     * Format: [uint32 length][ASCII type string][object data...]
     * This is how the original text parser works - it finds type names
     * and reads subsequent data based on the type. */
    size_t pos = 0;
    while (pos + 4 < size) {
        uint32_t slen = *(const uint32_t *)(data + pos);
        
        /* Valid object type strings: 2-80 chars, ASCII, followed by valid data */
        if (slen >= 2 && slen <= 80 && pos + 4 + slen <= size) {
            const char *candidate = (const char *)(data + pos + 4);
            
            /* Check if all chars in the string are valid ASCII */
            int valid = 1;
            for (uint32_t i = 0; i < slen; i++) {
                char c = candidate[i];
                if ((c < 32 || c > 126) && c != '\0') { valid = 0; break; }
            }
            
            if (valid && is_primary_type(candidate)) {
                /* Found a valid object type string */
                size_t obj_data_start = pos + 4 + slen;
                
                /* Skip past NUL padding (string may be NUL-padded to alignment) */
                size_t data_pos = obj_data_start;
                while (data_pos < size && data[data_pos] == '\0') data_pos++;
                
                /* Read position (3 floats = 12 bytes) after the type string */
                float x = 0, y = 0, z = 0;
                if (data_pos + 12 <= size) {
                    const float *fptr = (const float *)(data + data_pos);
                    x = fptr[0]; y = fptr[1]; z = fptr[2];
                }
                
                add_object(level, candidate, x, y, z, data, size, data_pos);
                
                /* Advance past this object */
                pos = obj_data_start;
                continue;
            }
        }
        pos++;
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
    if (level->objects) {
        for (int i = 0; i < level->object_count; i++) {
            if (level->objects[i].indices) free(level->objects[i].indices);
        }
        free(level->objects);
    }
    if (level->geometry_data) free(level->geometry_data);
    free(level);
}