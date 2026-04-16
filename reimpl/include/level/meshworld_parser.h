/*
 * MESHWORLD Parser - Public API v5
 * Parses Hamsterball level files (.MESHWORLD) — binary format
 */

#ifndef MESHWORLD_PARSER_H
#define MESHWORLD_PARSER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    MW_OBJ_MESHCILLION,
    MW_OBJ_RND,
    MW_OBJ_BALLPATH,
    MW_OBJ_BCMBALL,
    MW_OBJ_BADBALL,
    MW_OBJ_AEXPLODEHELPER,
    MW_OBJ_FOLLOWBALLSPOT,
    MW_OBJ_STANDS,       /* Arena stands/spectator area */
} mw_obj_type_t;

/* Vector types */
typedef struct { float x, y, z; } mw_vec3_t;
typedef struct { float u, v; } mw_vec2_t;

/* Vertex format (32 bytes per vertex, matches original binary format) */
typedef struct {
    float x, y, z;       /* Position */
    float nx, ny, nz;    /* Normal */
    float u, v;           /* Texture coordinates */
} mw_vertex_t;

/* Material (0x7C bytes in original, key fields here) */
typedef struct {
    char name[256];       /* Material name */
    uint32_t face_start;  /* First face index */
    uint32_t face_count;  /* Number of faces */
    uint32_t unk3, unk4, unk5, unk6;
    float ambient[4];     /* RGBA */
    float diffuse[4];     /* RGBA */
    float specular[4];    /* RGBA */
    float shine;          /* Shininess */
    int has_reflective;
    int has_texture;
    char texture[256];    /* Texture filename */
} mw_material_t;

/* Face data (placeholder for now) */
typedef struct {
    float unk0, unk1, unk2;
} mw_face_t;

/* Mesh buffer (geometry submesh) */
typedef struct {
    char name[256];
    uint32_t face_count;
    mw_face_t *faces;
} mw_mesh_buffer_t;

/* Object structure */
typedef struct {
    mw_obj_type_t type;
    char type_string[256];
    
    /* Position */
    mw_vec3_t position;
    
    /* Rotation (quaternion or euler) */
    float rot_x, rot_y, rot_z, rot_w;
    
    /* Transform/material data */
    float transform[8];
    float diffuse[4];
    float ambient[4];
    float size_param;
    uint32_t flags[2];
    
    /* Texture reference */
    char texture[256];
    
    /* Embedded properties */
    float modifier_value;
    char modifier_name[256];
    
    /* Face index data */
    int num_indices;
    uint32_t *indices;
} mw_object_t;

/* Level structure */
typedef struct mw_level_t {
    char name[256];
    int object_count;
    int object_capacity;
    mw_object_t *objects;
    
    /* Materials */
    int material_count;
    mw_material_t *materials;
    
    /* Mesh buffers (geometry submeshes) */
    uint32_t mesh_buffer_count;
    mw_mesh_buffer_t *mesh_buffers;
    
    /* Vertex array (32 bytes per vertex from Section 5) */
    int vertex_count;
    mw_vertex_t *vertices;
    
    /* Bounding box */
    float bbox_min_x, bbox_min_y, bbox_min_z;
    float bbox_max_x, bbox_max_y, bbox_max_z;
    
    /* Raw geometry reference */
    int geometry_size;
    uint8_t *geometry_data;
} mw_level_t;

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