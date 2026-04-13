/* Quick test for MESH parser */
#include <stdio.h>
#include "level/mesh_parser.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.MESH>\n", argv[0]);
        return 1;
    }
    mesh_model_t *model = mesh_parse_file(argv[1]);
    if (!model) {
        fprintf(stderr, "Failed to parse %s\n", argv[1]);
        return 1;
    }
    printf("Name: %s\n", model->name);
    printf("Vertices: %d\n", model->vertex_count);
    printf("Texture: %s\n", model->material.texture);
    printf("Material: ambient=(%.2f,%.2f,%.2f,%.2f) diffuse=(%.2f,%.2f,%.2f,%.2f)\n",
           model->material.ambient[0], model->material.ambient[1],
           model->material.ambient[2], model->material.ambient[3],
           model->material.diffuse[0], model->material.diffuse[1],
           model->material.diffuse[2], model->material.diffuse[3]);
    printf("Shine: %.2f\n", model->material.shine);
    printf("BBox: X[%.2f,%.2f] Y[%.2f,%.2f] Z[%.2f,%.2f]\n",
           model->min_x, model->max_x, model->min_y, model->max_y,
           model->min_z, model->max_z);
    if (model->vertex_count > 0) {
        printf("First vertex: pos=(%.3f,%.3f,%.3f) norm=(%.3f,%.3f,%.3f) uv=(%.4f,%.4f)\n",
               model->vertices[0].x, model->vertices[0].y, model->vertices[0].z,
               model->vertices[0].nx, model->vertices[0].ny, model->vertices[0].nz,
               model->vertices[0].u, model->vertices[0].v);
    }
    mesh_free(model);
    return 0;
}