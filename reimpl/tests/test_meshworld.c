/* test_meshworld.c - Test MESHWORLD parser against all original levels */
#include "test_helpers.h"
#include "level/meshworld_parser.h"
#include "core/filesys.h"
#include <dirent.h>
#include <sys/stat.h>

/* Try multiple possible asset locations */
static const char *ASSET_DIRS[] = {
    "../originals/installed/extracted",
    "../../originals/installed/extracted",
    "/home/evan/hamsterball-re/originals/installed/extracted",
    NULL
};

static const char *find_assets(void) {
    for (int i = 0; ASSET_DIRS[i]; i++) {
        char test[512];
        snprintf(test, sizeof(test), "%s/Levels", ASSET_DIRS[i]);
        DIR *d = opendir(test);
        if (d) { closedir(d); return ASSET_DIRS[i]; }
    }
    return NULL;
}

static int count_files_in_dir(const char *dir_path, const char *ext) {
    DIR *d = opendir(dir_path);
    if (!d) return 0;
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (strstr(ent->d_name, ext)) count++;
    }
    closedir(d);
    return count;
}

/* ===== Unit Tests ===== */

TEST(test_meshworld_parse_file_not_found) {
    mw_level_t *level = meshworld_parse_file("/nonexistent/path/Level999.MESHWORLD");
    ASSERT_NULL(level);
}

TEST(test_meshworld_parse_empty_data) {
    mw_level_t *level = meshworld_parse(NULL, 0);
    ASSERT_NULL(level);
}

TEST(test_meshworld_parse_tiny_data) {
    uint8_t tiny[] = {0x00, 0x00, 0x00, 0x00};
    mw_level_t *level = meshworld_parse(tiny, sizeof(tiny));
    /* Should either return NULL or a level with 0 objects - either is OK */
    if (level) meshworld_free(level);
}

TEST(test_meshworld_parse_level1) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/Levels/Level1.MESHWORLD", assets);
    mw_level_t *level = meshworld_parse_file(path);
    ASSERT_NOT_NULL(level);
    ASSERT_GT(level->object_count, 0);
    /* Level1 should have at least one START object */
    int starts = 0;
    for (int i = 0; i < level->object_count; i++) {
        if (level->objects[i].type == MW_OBJ_START) starts++;
    }
    ASSERT_GT(starts, 0);
    meshworld_free(level);
}

TEST(test_all_87_levels_parse) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char levels_dir[512];
    snprintf(levels_dir, sizeof(levels_dir), "%s/Levels", assets);
    
    DIR *d = opendir(levels_dir);
    ASSERT_NOT_NULL(d);
    
    int parsed = 0, total = 0, objects_total = 0;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!strstr(ent->d_name, ".MESHWORLD")) continue;
        total++;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", levels_dir, ent->d_name);
        mw_level_t *level = meshworld_parse_file(path);
        if (level && level->object_count > 0) {
            parsed++;
            objects_total += level->object_count;
        }
        if (level) meshworld_free(level);
    }
    closedir(d);
    
    printf("(%d/%d levels, %d total objects) ", parsed, total, objects_total);
    ASSERT_EQ(parsed, total);
    ASSERT_EQ(total, 87);
}

TEST(test_arena_levels_exist_and_parse) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    
    const char *arenas[] = {
        "Arena-WarmUp", "Arena-Beginner", "Arena-Intermediate", "Arena-Dizzy",
        "Arena-Tower", "Arena-Up", "Arena-Expert", "Arena-Odd",
        "Arena-Glass", "Arena-Impossible", "Arena-Master", "Arena-Sky",
        "Arena-Toob", "Arena-Wobbly", "Arena-Cascade",
        NULL
    };
    
    for (int i = 0; arenas[i]; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/Levels/%s.MESHWORLD", assets, arenas[i]);
        mw_level_t *level = meshworld_parse_file(path);
        if (!level) { g_tests_fail++; printf("FAIL: arena %s is NULL\n", arenas[i]); return; }
        ASSERT_GT(level->object_count, 0);
        meshworld_free(level);
    }
}

TEST(test_level_objects_have_valid_positions) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/Levels/Level1.MESHWORLD", assets);
    mw_level_t *level = meshworld_parse_file(path);
    ASSERT_NOT_NULL(level);
    
    int valid = 0;
    for (int i = 0; i < level->object_count; i++) {
        mw_object_t *obj = &level->objects[i];
        /* Positions should be reasonable (-10000 to 10000) */
        if (fabs(obj->position.x) < 10000.0f &&
            fabs(obj->position.y) < 10000.0f &&
            fabs(obj->position.z) < 10000.0f) {
            valid++;
        }
    }
    ASSERT_GT(valid, 0);
    meshworld_free(level);
}

TEST(test_racedata_xml_exists) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/RaceData.xml", assets);
    ASSERT_TRUE(filesys_file_exists(path));
}

TEST(test_jukebox_xml_exists) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/Jukebox.xml", assets);
    ASSERT_TRUE(filesys_file_exists(path));
}

/* ===== Filesystem Tests ===== */

TEST(test_filesys_init) {
    filesys_t fs;
    filesys_init(&fs, "/path/to/Hamsterball.exe");
    ASSERT_STR_EQ(fs.base_path, "/path/to");
}

TEST(test_filesys_init_no_slash) {
    filesys_t fs;
    filesys_init(&fs, "Hamsterball.exe");
    ASSERT_STR_EQ(fs.base_path, ".");
}

TEST(test_filesys_resolve) {
    filesys_t fs;
    filesys_init(&fs, "/games/hamsterball");
    char out[512];
    size_t len = filesys_resolve(&fs, "Levels", "Level1", "MESHWORLD", out, sizeof(out));
    ASSERT_GT(len, 0);
    ASSERT_STR_EQ(out, "/games/hamsterball/Levels/Level1.MESHWORLD");
}

TEST(test_filesys_resolve_texture) {
    filesys_t fs;
    filesys_init(&fs, "/games/hamsterball");
    char out[512];
    filesys_resolve(&fs, "Textures", "HamsterBall", "png", out, sizeof(out));
    ASSERT_STR_EQ(out, "/games/hamsterball/Textures/HamsterBall.png");
}

TEST(test_filesys_read_file) {
    const char *assets = find_assets();
    if (!assets) { printf("SKIP (no assets)"); return; }
    char path[512];
    snprintf(path, sizeof(path), "%s/RaceData.xml", assets);
    uint8_t *data = NULL;
    size_t size = 0;
    int rc = filesys_read_file(path, &data, &size);
    ASSERT_EQ(rc, 0);
    ASSERT_NOT_NULL(data);
    ASSERT_GT(size, 0);
    /* Should start with XML */
    ASSERT_TRUE(data[0] == '<' || data[0] == 0xEF /* BOM */);
    free(data);
}

void test_all(void) {
    /* Filesys tests first (no assets needed) */
    RUN_TEST(test_filesys_init);
    RUN_TEST(test_filesys_init_no_slash);
    RUN_TEST(test_filesys_resolve);
    RUN_TEST(test_filesys_resolve_texture);
    
    /* MESHWORLD parser tests */
    RUN_TEST(test_meshworld_parse_file_not_found);
    RUN_TEST(test_meshworld_parse_empty_data);
    RUN_TEST(test_meshworld_parse_tiny_data);
    
    /* Asset-dependent tests */
    RUN_TEST(test_meshworld_parse_level1);
    RUN_TEST(test_all_87_levels_parse);
    RUN_TEST(test_arena_levels_exist_and_parse);
    RUN_TEST(test_level_objects_have_valid_positions);
    RUN_TEST(test_racedata_xml_exists);
    RUN_TEST(test_jukebox_xml_exists);
    RUN_TEST(test_filesys_read_file);
}

TEST_MAIN()