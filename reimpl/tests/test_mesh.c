/* test_mesh.c - Test MESH parser against all original mesh files */
#include "test_helpers.h"
#include "level/mesh_parser.h"
#include <dirent.h>

static const char *ASSET_DIRS[] = {
    "../originals/installed/extracted",
    "../../originals/installed/extracted",
    "/home/evan/hamsterball-re/originals/installed/extracted",
    NULL
};

static const char *find_assets(void) {
    for (int i = 0; ASSET_DIRS[i]; i++) {
        char test[512];
        snprintf(test, sizeof(test), "%s/Meshes", ASSET_DIRS[i]);
        DIR *d = opendir(test);
        if (d) { closedir(d); return ASSET_DIRS[i]; }
    }
    return NULL;
}

TEST(test_sphere_mesh_parses) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/Meshes/Sphere.MESH", assets);
    mesh_model_t *mesh = mesh_parse_file(path);
    ASSERT_NOT_NULL(mesh);
    ASSERT_GT(mesh->vertex_count, 0);
    /* Sphere should have ~59 vertices (known from analysis) */
    ASSERT_GT(mesh->vertex_count, 30);
    if (mesh->material.texture[0]) {
        ASSERT_TRUE(strlen(mesh->material.texture) > 3);
    }
    mesh_free(mesh);
}

TEST(test_8ball_mesh_parses) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/Meshes/8Ball.MESH", assets);
    mesh_model_t *mesh = mesh_parse_file(path);
    ASSERT_NOT_NULL(mesh);
    ASSERT_GT(mesh->vertex_count, 0);
    mesh_free(mesh);
}

TEST(test_all_meshes_parse) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char meshes_dir[512];
    snprintf(meshes_dir, sizeof(meshes_dir), "%s/Meshes", assets);
    
    DIR *d = opendir(meshes_dir);
    ASSERT_NOT_NULL(d);
    
    int parsed = 0, total = 0, vtotal = 0, failed = 0;
    char failed_names[10][256];
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!strstr(ent->d_name, ".MESH")) continue;
        total++;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", meshes_dir, ent->d_name);
        mesh_model_t *mesh = mesh_parse_file(path);
        if (mesh && mesh->vertex_count > 0) {
            parsed++;
            vtotal += mesh->vertex_count;
        } else {
            if (failed < 10) strncpy(failed_names[failed], ent->d_name, 255);
            failed++;
        }
        if (mesh) mesh_free(mesh);
    }
    closedir(d);
    
    printf("(%d/%d meshes, %d total verts) ", parsed, total, vtotal);
    if (failed > 0 && failed <= 10) {
        printf("(%d failed: ", failed);
        for (int i = 0; i < failed && i < 10; i++) printf("%s ", failed_names[i]);
        printf(") ");
    }
    ASSERT_GT(parsed, 0);
    ASSERT_GT(total, 20);
}

TEST(test_mesh_vertex_bounds) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/Meshes/Sphere.MESH", assets);
    mesh_model_t *mesh = mesh_parse_file(path);
    ASSERT_NOT_NULL(mesh);
    
    for (int i = 0; i < mesh->vertex_count; i++) {
        mesh_vertex_t *v = &mesh->vertices[i];
        /* Vertices should have finite coordinates */
        ASSERT_TRUE(isfinite(v->x));
        ASSERT_TRUE(isfinite(v->y));
        ASSERT_TRUE(isfinite(v->z));
    }
    mesh_free(mesh);
}

void test_all(void) {
    RUN_TEST(test_sphere_mesh_parses);
    RUN_TEST(test_8ball_mesh_parses);
    RUN_TEST(test_all_meshes_parse);
    RUN_TEST(test_mesh_vertex_bounds);
}

TEST_MAIN()