/*
 * Hamsterball MESH Binary Parser v1
 * Parses .MESH 3D model files (Hamster, Sphere, etc.)
 *
 * Format:
 *   [4] version (always 1)
 *   [4] name_length
 *   [name_length] name string (null-terminated, may have padding)
 *   [88] material data (ambient RGBA, diffuse RGBA, specular RGBA, shininess, etc.)
 *   [4] texture_name_length
 *   [texture_name_length] texture filename
 *   [4] vertex_count
 *   [4] unknown (material group count? always small)
 *   [vertex_count * 32] vertex array
 *     each vertex: [float x,y,z][float nx,ny,nz][float u,v] = 32 bytes
 *   [remaining] face/triangle data + additional geometry
 */

#ifndef MESH_PARSER_H
#define MESH_PARSER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
} mesh_vertex_t;

typedef struct {
    float ambient[4];    /* RGBA */
    float diffuse[4];    /* RGBA */
    float specular[4];   /* RGB + alpha */
    float shine;         /* Shininess */
    float shadow_bias;   /* Usually 0 */
    float shadow_scale;   /* Usually 0 */
    char texture[256];   /* Texture filename */
} mesh_material_t;

typedef struct {
    char name[256];
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
    int vertex_count;
    mesh_vertex_t *vertices;
    mesh_material_t material;
} mesh_model_t;

/* Parse a .MESH file from memory */
mesh_model_t *mesh_parse(const uint8_t *data, size_t size);

/* Parse a .MESH file from path */
mesh_model_t *mesh_parse_file(const char *path);

/* Free a parsed model */
void mesh_free(mesh_model_t *model);

#ifdef __cplusplus
}
#endif

#endif /* MESH_PARSER_H */