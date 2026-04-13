/*
 * Hamsterball Binary Mesh/MeshWorld Parser v4
 * Based on Ghidra decompilation of FUN_004629E0 (binary mesh loader)
 * and confirmed format from actual level file analysis
 * 
 * Binary MESHWORLD format:
 *   Section 1: Materials (material_count + material data)
 *   Section 2: Mesh Buffers (geometry submeshes)
 *   Section 3: Game Objects (collision surfaces, spawn points, etc.)
 *   Section 4: Bounding Box (6 floats)
 *   Section 5: Vertex Array (count * 32 bytes)
 *
 * String format in binary: [uint32 length][length bytes data]
 */

#ifndef MW_PARSER_V4_H
#define MW_PARSER_V4_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Vertex format: 32 bytes */
typedef struct {
    float x, y, z;      /* position */
    float nx, ny, nz;    /* normal */
    float u, v;          /* texture coordinates */
} mw_vertex_t;

/* Material */
typedef struct {
    char name[256];
    char texture[256];
    int face_start;
    int face_count;
    float ambient[4];    /* RGBA */
    float diffuse[4];    /* RGBA */
    float specular[4];  /* RGBA */
    float shine;
    int has_reflective;
    int has_texture;
    int is_transparent;
} mw_material_t;

/* Game Object (from Section 3) */
typedef struct {
    int type;            /* 0=straight, others=special */
    float position[3];   /* x, y, z */
    float size_params[3];/* from vtable calls */
    float scale[3];      /* transform */
} mw_gameobj_t;

/* Parsed level */
typedef struct {
    /* Section 1: Materials */
    int material_count;
    mw_material_t *materials;
    
    /* Section 2: Mesh buffers - for now just store raw data */
    int mesh_buffer_count;
    
    /* Section 3: Game objects */
    int object_count;
    mw_gameobj_t *objects;
    
    /* Section 4: Bounding box */
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
    
    /* Section 5: Vertex array */
    int vertex_count;
    mw_vertex_t *vertices;
    
    /* Raw object data (for fallback string-scanning) */
    int raw_object_count;
    struct {
        char type_string[256];
        char texture[256];
        float position[3];
        float rotation[4];
        int type_enum;
    } *raw_objects;
} mw_level_v4_t;

/* Parse binary MESHWORLD file */
mw_level_v4_t *mw_parse_v4(const uint8_t *data, size_t size);
mw_level_v4_t *mw_parse_v4_file(const char *path);
void mw_free_v4(mw_level_v4_t *level);

#ifdef __cplusplus
}
#endif

#endif /* MW_PARSER_V4_H */