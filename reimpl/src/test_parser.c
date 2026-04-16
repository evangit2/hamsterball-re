/* Test MESHWORLD parser without D3D8 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "level/meshworld_parser.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.MESHWORLD>\n", argv[0]);
        return 1;
    }
    
    mw_level_t *level = meshworld_parse_file(argv[1]);
    if (!level) {
        fprintf(stderr, "Failed to parse %s\n", argv[1]);
        return 1;
    }
    
    printf("=== Level: %s ===\n", argv[1]);
    printf("Materials:  %d\n", level->material_count);
    printf("MeshBufs:   %u\n", level->mesh_buffer_count);
    printf("Objects:    %d\n", level->object_count);
    printf("Vertices:   %d\n", level->vertex_count);
    printf("BBox:       (%.1f,%.1f,%.1f) - (%.1f,%.1f,%.1f)\n",
           level->bbox_min_x, level->bbox_min_y, level->bbox_min_z,
           level->bbox_max_x, level->bbox_max_y, level->bbox_max_z);
    
    if (level->vertex_count > 0) {
        printf("\nFirst 5 vertices:\n");
        for (int i = 0; i < 5 && i < level->vertex_count; i++) {
            mw_vertex_t *v = &level->vertices[i];
            printf("  [%d] pos=(%.2f,%.2f,%.2f) n=(%.3f,%.3f,%.3f) uv=(%.4f,%.4f)\n",
                   i, v->x, v->y, v->z, v->nx, v->ny, v->nz, v->u, v->v);
        }
    }
    
    if (level->object_count > 0) {
        printf("\nObjects:\n");
        for (int i = 0; i < level->object_count && i < 20; i++) {
            mw_object_t *o = &level->objects[i];
            printf("  [%d] type=%d '%s' pos=(%.2f,%.2f,%.2f)\n",
                   i, o->type, o->type_string, o->position.x, o->position.y, o->position.z);
        }
    }
    
    if (level->material_count > 0) {
        printf("\nMaterials:\n");
        for (int i = 0; i < level->material_count && i < 20; i++) {
            mw_material_t *m = &level->materials[i];
            printf("  [%d] '%s' faces=%d tex='%s'\n",
                   i, m->name, m->face_count, m->texture);
        }
    }
    
    meshworld_free(level);
    return 0;
}