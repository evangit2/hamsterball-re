/* test_quick.c - Quick sanity tests, no asset scanning */
#include "test_helpers.h"
#include "core/filesys.h"
#include "level/meshworld_parser.h"
#include "level/mesh_parser.h"

TEST(test_filesys_init) {
    filesys_t fs;
    filesys_init(&fs, "/path/to/Hamsterball.exe");
    ASSERT_STR_EQ(fs.base_path, "/path/to");
}

TEST(test_filesys_resolve) {
    filesys_t fs;
    filesys_init(&fs, "/games/hb/Hamsterball.exe");
    char out[512];
    filesys_resolve(&fs, "Levels", "Level1", "MESHWORLD", out, sizeof(out));
    ASSERT_STR_EQ(out, "/games/hb/Levels/Level1.MESHWORLD");
}

TEST(test_filesys_null) {
    filesys_t fs;
    filesys_init(&fs, "Hamsterball.exe");
    ASSERT_STR_EQ(fs.base_path, ".");
}

TEST(test_meshworld_null_data) {
    mw_level_t *level = meshworld_parse(NULL, 0);
    ASSERT_NULL(level);
}

TEST(test_meshworld_tiny_data) {
    uint8_t tiny[] = {0,0,0,0};
    mw_level_t *level = meshworld_parse(tiny, sizeof(tiny));
    if (level) meshworld_free(level);
}

TEST(test_meshworld_level1) {
    mw_level_t *level = meshworld_parse_file("/home/evan/hamsterball-re/originals/installed/extracted/Levels/Level1.MESHWORLD");
    ASSERT_NOT_NULL(level);
    ASSERT_GT(level->object_count, 0);
    int starts = 0;
    for (int i = 0; i < level->object_count; i++) {
        if (level->objects[i].type == MW_OBJ_START) starts++;
    }
    ASSERT_GT(starts, 0);
    meshworld_free(level);
}

TEST(test_mesh_world_file_notfound) {
    mw_level_t *level = meshworld_parse_file("/nonexistent/Level999.MESHWORLD");
    ASSERT_NULL(level);
}

TEST(test_mesh_sphere) {
    mesh_model_t *m = mesh_parse_file("/home/evan/hamsterball-re/originals/installed/extracted/Meshes/Sphere.MESH");
    ASSERT_NOT_NULL(m);
    ASSERT_GT(m->vertex_count, 30);
    mesh_free(m);
}

TEST(test_mesh_8ball) {
    mesh_model_t *m = mesh_parse_file("/home/evan/hamsterball-re/originals/installed/extracted/Meshes/8Ball.MESH");
    ASSERT_NOT_NULL(m);
    ASSERT_GT(m->vertex_count, 0);
    mesh_free(m);
}

TEST(test_filesys_read_file) {
    uint8_t *data = NULL; size_t size = 0;
    int rc = filesys_read_file("/home/evan/hamsterball-re/originals/installed/extracted/RaceData.xml", &data, &size);
    ASSERT_EQ(rc, 0);
    ASSERT_NOT_NULL(data);
    ASSERT_GT(size, 0);
    free(data);
}

void test_all(void) {
    RUN_TEST(test_filesys_init);
    RUN_TEST(test_filesys_resolve);
    RUN_TEST(test_filesys_null);
    RUN_TEST(test_meshworld_null_data);
    RUN_TEST(test_meshworld_tiny_data);
    RUN_TEST(test_meshworld_level1);
    RUN_TEST(test_mesh_world_file_notfound);
    RUN_TEST(test_mesh_sphere);
    RUN_TEST(test_mesh_8ball);
    RUN_TEST(test_filesys_read_file);
}

TEST_MAIN()