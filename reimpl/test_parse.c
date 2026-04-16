/* Quick standalone test: parse a MESHWORLD file and print vertex/object stats */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "level/meshworld_parser.h"

int main(int argc, char **argv) {
    const char *path = "../originals/installed/extracted/Levels/Arena-WarmUp.MESHWORLD";
    if (argc > 1) path = argv[1];
    
    printf("Parsing: %s\n", path);
    mw_level_t *level = meshworld_parse_file(path);
    if (!level) { fprintf(stderr, "FAILED\n"); return 1; }
    
    printf("Objects: %d\n", level->object_count);
    printf("Vertices: %d\n", level->vertex_count);
    printf("Bounds: (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)\n",
        level->bounds_min.x, level->bounds_min.y, level->bounds_min.z,
        level->bounds_max.x, level->bounds_max.y, level->bounds_max.z);
    printf("BG color: (%.3f,%.3f,%.3f)\n", level->bg_color.x, level->bg_color.y, level->bg_color.z);
    printf("Ambient: (%.3f,%.3f,%.3f)\n", level->ambient_color.x, level->ambient_color.y, level->ambient_color.z);
    
    if (level->vertices && level->vertex_count > 0) {
        printf("\nFirst 10 vertices:\n");
        int n = level->vertex_count < 10 ? level->vertex_count : 10;
        for (int i = 0; i < n; i++) {
            mw_vertex_t *v = &level->vertices[i];
            printf("  [%d] pos=(%.2f,%.2f,%.2f) n=(%.2f,%.2f,%.2f) uv=(%.3f,%.3f)\n",
                i, v->x, v->y, v->z, v->nx, v->ny, v->nz, v->u, v->v);
        }
        printf("\nLast 3 vertices:\n");
        for (int i = level->vertex_count - 3; i < level->vertex_count; i++) {
            mw_vertex_t *v = &level->vertices[i];
            printf("  [%d] pos=(%.2f,%.2f,%.2f)\n", i, v->x, v->y, v->z);
        }
        
        /* Compute bounding box from vertices */
        float vmin[3] = {1e30,1e30,1e30}, vmax[3] = {-1e30,-1e30,-1e30};
        for (int i = 0; i < level->vertex_count; i++) {
            mw_vertex_t *v = &level->vertices[i];
            if (v->x < vmin[0]) vmin[0] = v->x;
            if (v->y < vmin[1]) vmin[1] = v->y;
            if (v->z < vmin[2]) vmin[2] = v->z;
            if (v->x > vmax[0]) vmax[0] = v->x;
            if (v->y > vmax[1]) vmax[1] = v->y;
            if (v->z > vmax[2]) vmax[2] = v->z;
        }
        printf("\nVertex bbox: (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)\n",
            vmin[0], vmin[1], vmin[2], vmax[0], vmax[1], vmax[2]);
    }
    
    printf("\nObjects detail:\n");
    for (int i = 0; i < level->object_count; i++) {
        mw_object_t *obj = &level->objects[i];
        printf("  [%d] %s type=%d pos=(%.1f,%.1f,%.1f) tex=%s\n",
            i, obj->type_string, obj->type,
            obj->position.x, obj->position.y, obj->position.z,
            obj->texture[0] ? obj->texture : "(none)");
    }
    
    meshworld_free(level);
    return 0;
}