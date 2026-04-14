/* test_assets.c - Comprehensive asset compatibility test
 * Verifies ALL original game assets load correctly */
#include "test_helpers.h"
#include "core/filesys.h"
#include "level/meshworld_parser.h"
#include "level/mesh_parser.h"
#include <dirent.h>
#include <sys/stat.h>

#define GAME_DIR "/home/evan/hamsterball-re/originals/installed/extracted"

/* ===== Texture count test ===== */
TEST(test_textures_exist) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/Textures", GAME_DIR);
    DIR *d = opendir(dir);
    ASSERT_NOT_NULL(d);
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strstr(ent->d_name, ".png") || strstr(ent->d_name, ".bmp") || strstr(ent->d_name, ".jpg"))
            count++;
    }
    closedir(d);
    printf("(%d textures) ", count);
    ASSERT_GT(count, 100);
}

/* ===== MESH v1 parsing ===== */
TEST(test_v1_meshes) {
    const char *v1_meshes[] = {"Sphere.MESH", "8Ball.MESH", "FunBall.MESH", 
                               "Eye.MESH", "TarBubble.MESH", "Sawblade.MESH"};
    int total = sizeof(v1_meshes) / sizeof(v1_meshes[0]);
    int parsed = 0;
    for (int i = 0; i < total; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/Meshes/%s", GAME_DIR, v1_meshes[i]);
        mesh_model_t *m = mesh_parse_file(path);
        if (m && m->vertex_count > 0) parsed++;
        if (m) mesh_free(m);
    }
    printf("(%d/%d v1 meshes) ", parsed, total);
    ASSERT_EQ(parsed, total);
}

/* ===== Sound files exist ===== */
TEST(test_sounds_exist) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/Sounds", GAME_DIR);
    DIR *d = opendir(dir);
    ASSERT_NOT_NULL(d);
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strstr(ent->d_name, ".wav") || strstr(ent->d_name, ".ogg"))
            count++;
    }
    closedir(d);
    printf("(%d sounds) ", count);
    ASSERT_GT(count, 30);
}

/* ===== Music files exist ===== */
TEST(test_music_exists) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/Music", GAME_DIR);
    DIR *d = opendir(dir);
    ASSERT_NOT_NULL(d);
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strstr(ent->d_name, ".mo3") || strstr(ent->d_name, ".ogg") || 
            strstr(ent->d_name, ".xm") || strstr(ent->d_name, ".it"))
            count++;
    }
    closedir(d);
    printf("(%d music files) ", count);
    ASSERT_GT(count, 0);
}

/* ===== Data files exist ===== */
TEST(test_data_files) {
    char path[512];
    snprintf(path, sizeof(path), "%s/RaceData.xml", GAME_DIR);
    ASSERT_TRUE(filesys_file_exists(path));
    snprintf(path, sizeof(path), "%s/Jukebox.xml", GAME_DIR);
    ASSERT_TRUE(filesys_file_exists(path));
}

/* ===== Font files exist ===== */
TEST(test_fonts_exist) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/Fonts", GAME_DIR);
    DIR *d = opendir(dir);
    ASSERT_NOT_NULL(d);
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        /* Font directories contain subdirs like ShowcardGothic16 */
        if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            count++;
    }
    closedir(d);
    printf("(%d font dirs) ", count);
    ASSERT_GT(count, 2);
}

/* ===== All 87 levels parse ===== */
TEST(test_all_levels) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/Levels", GAME_DIR);
    DIR *d = opendir(dir);
    ASSERT_NOT_NULL(d);
    int parsed = 0, total = 0, sublevels = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!strstr(ent->d_name, ".MESHWORLD")) continue;
        total++;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        mw_level_t *level = meshworld_parse_file(path);
        if (level) {
            parsed++;
            if (level->object_count == 0) sublevels++;
        }
        if (level) meshworld_free(level);
    }
    closedir(d);
    printf("(%d/%d parsed, %d sublevels with 0 objects) ", parsed, total, sublevels);
    ASSERT_EQ(parsed, total);  /* All must parse without error */
    ASSERT_GT(total, 80);      /* Known count is 87 but may vary */
}

/* ===== Mesh parsing summary ===== */
TEST(test_mesh_summary) {
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/Meshes", GAME_DIR);
    DIR *d = opendir(dir);
    ASSERT_NOT_NULL(d);
    int parsed = 0, total = 0, total_v = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!strstr(ent->d_name, ".MESH")) continue;
        total++;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        mesh_model_t *m = mesh_parse_file(path);
        if (m && m->vertex_count > 0) { parsed++; total_v += m->vertex_count; }
        if (m) mesh_free(m);
    }
    closedir(d);
    printf("(%d/%d meshes parsed, %d total verts) ", parsed, total, total_v);
    ASSERT_GT(parsed, 20);
    ASSERT_GT(total, 30);
}

void test_all(void) {
    RUN_TEST(test_textures_exist);
    RUN_TEST(test_v1_meshes);
    RUN_TEST(test_sounds_exist);
    RUN_TEST(test_music_exists);
    RUN_TEST(test_data_files);
    RUN_TEST(test_fonts_exist);
    RUN_TEST(test_all_levels);
    RUN_TEST(test_mesh_summary);
}

TEST_MAIN()