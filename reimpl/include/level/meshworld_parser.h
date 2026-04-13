/*
 * MESHWORLD Parser - Public API
 * Parses Hamsterball level files (.MESHWORLD)
 */

#ifndef MESHWORLD_PARSER_H
#define MESHWORLD_PARSER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mw_level_t mw_level_t;
typedef struct mw_object_t mw_object_t;

/* Object types */
typedef enum {
    MW_OBJ_UNKNOWN = 0,
    MW_OBJ_START,
    MW_OBJ_FLAG,
    MW_OBJ_SAFESPOT,
    MW_OBJ_CAMERALOOKAT,
    MW_OBJ_PLATFORM,
    MW_OBJ_SINKPLATFORM,
    MW_OBJ_NODIZZY,
    MW_OBJ_GROWSOUND,
    MW_OBJ_GOAL,
    MW_OBJ_MESH,
    MW_OBJ_MESHCILLISION,
    MW_OBJ_RND,
    MW_OBJ_BALLPATH,
    MW_OBJ_BCMBALL,
    MW_OBJ_BADBALL,
    MW_OBJ_AEXPLODEHELPER,
    MW_OBJ_FOLLOWBALLSPOT,
} mw_obj_type_t;

/* Vector types */
typedef struct { float x, y, z; } mw_vec3_t;
typedef struct { float u, v; } mw_vec2_t;

/* Object structure */
struct mw_object_t {
    mw_obj_type_t type;
    char type_string[256];
    
    /* Position (first 3 floats after type string) */
    mw_vec3_t position;
    
    /* Rotation (4 values after position in simple objects) */
    float rot_x, rot_y, rot_z, rot_w;
    
    /* Transform/material data (for complex objects) */
    float transform[8];     /* Rotation/scale matrix */
    float diffuse[4];       /* Diffuse color RGB + alpha */
    float ambient[4];       /* Ambient/specular + alpha */
    float size_param;       /* Platform size or other parameter */
    uint32_t flags[2];      /* Object flags */
    
    /* Texture reference */
    char texture[256];
    
    /* Embedded properties (for NODIZZY etc.) */
    float modifier_value;
    char modifier_name[256];
    
    /* Face index data */
    int num_indices;
    uint32_t *indices;
};

/* Level structure */
struct mw_level_t {
    char name[256];
    int object_count;
    int object_capacity;
    mw_object_t *objects;       /* Heap-allocated, grows as needed */
    
    /* Geometry section (raw binary) */
    int geometry_size;
    uint8_t *geometry_data;
};

/* Parse a MESHWORLD file from memory */
mw_level_t *meshworld_parse(const uint8_t *data, size_t size);

/* Parse a MESHWOROLD file from path */
mw_level_t *meshworld_parse_file(const char *path);

/* Free a parsed level */
void meshworld_free(mw_level_t *level);

#ifdef __cplusplus
}
#endif

#endif /* MESHWORLD_PARSER_H */