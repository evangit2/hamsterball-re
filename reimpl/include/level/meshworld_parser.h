/*
 * MESHWORLD Parser v5 - Public API
 * Parses Hamsterball level files (.MESHWORLD) using official format spec
 * from Raptisoft's 3DS Max exporter source code.
 *
 * Definitive spec: docs/MESHWORLD_BINARY_FORMAT_OFFICIAL.md
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
    MW_OBJ_GOAL,        /* N: and E: prefixed objects */
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

/* Vertex structure (32 bytes) — matches official Raptisoft exporter exactly */
typedef struct {
    float x, y, z;          /* Position */
    float nx, ny, nz;       /* Normal */
    float u, v;             /* Texture coordinates */
} mw_vertex_t;

/* Object structure (ref points from Section 1) */
struct mw_object_t {
    mw_obj_type_t type;
    char type_string[256];
    
    /* Position (converted to D3D Y-up from Max Z-up) */
    mw_vec3_t position;
    
    /* Rotation */
    float rot_x, rot_y, rot_z, rot_w;
    
    /* Transform/material data */
    float transform[8];
    float diffuse[4];
    float ambient[4];
    float size_param;       /* Also shininess for materials */
    
    /* Texture reference */
    char texture[256];
    
    /* Embedded properties */
    float modifier_value;
    char modifier_name[256];
    
    /* Face index data (legacy) */
    int num_indices;
    uint32_t *indices;
};

/* Level structure */
struct mw_level_t {
    char name[256];
    int object_count;
    int object_capacity;
    mw_object_t *objects;
    
    /* Background & ambient colors (Section 4) */
    mw_vec3_t bg_color;
    mw_vec3_t ambient_color;
    
    /* Global vertex buffer (Section 5) — THE geometry */
    int vertex_count;
    mw_vertex_t *vertices;
    
    /* Bounding box (root of octree, Section 6) */
    mw_vec3_t bounds_min;
    mw_vec3_t bounds_max;
    
    /* Geometry data (raw binary, for octree walk later) */
    int geometry_size;
    uint8_t *geometry_data;
};

/* Parse a MESHWORLD file from memory */
mw_level_t *meshworld_parse(const uint8_t *data, size_t size);

/* Parse a MESHWORLD file from path */
mw_level_t *meshworld_parse_file(const char *path);

/* Free a parsed level */
void meshworld_free(mw_level_t *level);

#ifdef __cplusplus
}
#endif

#endif /* MESHWORLD_PARSER_H */