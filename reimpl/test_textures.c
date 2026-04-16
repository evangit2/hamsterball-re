#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "level/meshworld_parser.h"

int main(int argc, char *argv[]) {
    const char *path = "build-win64/Levels/WarmUp.MESHWORLD";
    if (argc > 1) path = argv[1];
    
    mw_level_t *level = meshworld_parse_file(path);
    if (!level) {
        fprintf(stderr, "Failed to parse %s\n", path);
        return 1;
    }
    
    printf("=== Level: %s ===\n", path);
    printf("Vertices: %d\n", level->vertex_count);
    printf("Objects: %d\n", level->object_count);
    printf("Geoms: %d\n", level->geom_count);
    printf("Indices: %d\n", level->index_count);
    printf("\n--- Distinct Textures in Geoms ---\n");
    
    char seen[64][256];
    int seen_count = 0;
    
    for (int g = 0; g < level->geom_count; g++) {
        mw_geom_t *geom = &level->geoms[g];
        if (geom->has_texture && geom->texture[0]) {
            int found = 0;
            for (int s = 0; s < seen_count; s++) {
                if (strcmp(seen[s], geom->texture) == 0) { found = 1; break; }
            }
            if (!found && seen_count < 64) {
                strncpy(seen[seen_count], geom->texture, 255);
                seen_count++;
                printf("  %d: \"%s\" (from geom %d: \"%s\")\n", seen_count, geom->texture, g, geom->name);
            }
        }
    }
    
    printf("\n--- Geom Details ---\n");
    for (int g = 0; g < level->geom_count && g < 30; g++) {
        mw_geom_t *geom = &level->geoms[g];
        printf("Geom %d: name=\"%s\" has_tex=%d tex=\"%s\" strips=%d\n", 
               g, geom->name, geom->has_texture, geom->texture, geom->strip_count);
        printf("  diffuse=(%.2f,%.2f,%.2f,%.2f)\n", geom->diffuse[0], geom->diffuse[1], geom->diffuse[2], geom->diffuse[3]);
        printf("  ambient=(%.2f,%.2f,%.2f,%.2f)\n", geom->ambient[0], geom->ambient[1], geom->ambient[2], geom->ambient[3]);
        for (int s = 0; s < geom->strip_count; s++) {
            printf("  strip %d: tri_count=%d vertex_offset=%d\n", s, geom->strips[s].tri_count, geom->strips[s].vertex_offset);
        }
    }
    
    /* Check UV range */
    float umin=1e30, umax=-1e30, vmin=1e30, vmax=-1e30;
    for (int i = 0; i < level->vertex_count; i++) {
        if (level->vertices[i].u < umin) umin = level->vertices[i].u;
        if (level->vertices[i].u > umax) umax = level->vertices[i].u;
        if (level->vertices[i].v < vmin) vmin = level->vertices[i].v;
        if (level->vertices[i].v > vmax) vmax = level->vertices[i].v;
    }
    printf("\n--- UV Range ---\n");
    printf("U: %f to %f\n", umin, umax);
    printf("V: %f to %f\n", vmin, vmax);
    
    /* Check normal range */
    float nmin=-1e30, nmax=1e30;
    int nn_zero = 0;
    for (int i = 0; i < level->vertex_count && i < 100; i++) {
        mw_vertex_t *v = &level->vertices[i];
        float nlen = sqrtf(v->nx*v->nx + v->ny*v->ny + v->nz*v->nz);
        if (nlen < 0.01f) nn_zero++;
    }
    printf("Normals: first 100 vertices, %d have ~zero length (degenerate)\n", nn_zero);
    
    meshworld_free(level);
    return 0;
}
