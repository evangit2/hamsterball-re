/* test_xml_parsers.c - Test Jukebox.xml and RaceData.xml parsing */
#include "test_helpers.h"
#include "audio/jukebox.h"
#include "game/race_data.h"

#define GAME_DIR "/home/evan/hamsterball-re/originals/installed/extracted"

TEST(test_jukebox_load) {
    jukebox_t jb;
    char path[512];
    snprintf(path, sizeof(path), "%s/Jukebox.xml", GAME_DIR);
    int rc = jukebox_load(&jb, path);
    ASSERT_EQ(rc, 0);
    ASSERT_GT(jb.song_count, 10);
    printf("(%d songs) ", jb.song_count);
    
    /* Verify specific songs */
    jukebox_song_t *theme = jukebox_title_theme(&jb);
    ASSERT_NOT_NULL(theme);
    ASSERT_TRUE(strcmp(theme->name, "Main Theme") == 0);
    
    jukebox_song_t *goal = jukebox_find(&jb, "Goal");
    ASSERT_NOT_NULL(goal);
    
    jukebox_free(&jb);
}

TEST(test_jukebox_hex_values) {
    jukebox_t jb;
    char path[512];
    snprintf(path, sizeof(path), "%s/Jukebox.xml", GAME_DIR);
    jukebox_load(&jb, path);
    
    /* Main Theme hex=02 */
    jukebox_song_t *theme = jukebox_title_theme(&jb);
    ASSERT_EQ(theme->hex_order, 0x02);
    
    /* Goal hex=6B */
    jukebox_song_t *goal = jukebox_find(&jb, "Goal!");
    ASSERT_NOT_NULL(goal);
    ASSERT_EQ(goal->hex_order, 0x6B);
    
    jukebox_free(&jb);
}

TEST(test_race_data_load) {
    race_data_set_t rds;
    char path[512];
    snprintf(path, sizeof(path), "%s/RaceData.xml", GAME_DIR);
    int rc = race_data_load(&rds, path);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(rds.race_count, 15);
    printf("(%d races) ", rds.race_count);
    
    /* Verify beginner race */
    race_data_t *beginner = race_data_find(&rds, "BEGINNERRACE");
    ASSERT_NOT_NULL(beginner);
    ASSERT_EQ(beginner->time_limit, 60);
    ASSERT_TRUE(beginner->par_time > 40.0f && beginner->par_time < 60.0f);
    ASSERT_TRUE(beginner->gold_time > 0);
    ASSERT_TRUE(beginner->silver_time > beginner->gold_time);
    ASSERT_TRUE(beginner->bronze_time > beginner->silver_time);
    
    race_data_free(&rds);
}

TEST(test_race_data_weasel_times) {
    race_data_set_t rds;
    char path[512];
    snprintf(path, sizeof(path), "%s/RaceData.xml", GAME_DIR);
    race_data_load(&rds, path);
    
    /* Weasel (developer) times should be the fastest */
    for (int i = 0; i < rds.race_count; i++) {
        race_data_t *r = &rds.races[i];
        ASSERT_LT(r->weasel_time + 0.1f, r->gold_time + 0.1f); /* weasel <= gold */
    }
    
    /* Specific known values: Beginner weasel=6.6, gold=7.6, silver=10.3, bronze=15.0 */
    race_data_t *b = race_data_find(&rds, "BEGINNERRACE");
    ASSERT_FEQ(b->weasel_time, 6.6f, 0.1f);
    ASSERT_FEQ(b->gold_time, 7.6f, 0.1f);
    ASSERT_FEQ(b->silver_time, 10.3f, 0.1f);
    ASSERT_FEQ(b->bronze_time, 15.0f, 0.1f);
    
    race_data_free(&rds);
}

TEST(test_race_data_notfound) {
    race_data_set_t rds;
    race_data_t *r = race_data_find(&rds, "NONEXISTENT");
    ASSERT_NULL(r);
}

void test_all(void) {
    RUN_TEST(test_jukebox_load);
    RUN_TEST(test_jukebox_hex_values);
    RUN_TEST(test_race_data_load);
    RUN_TEST(test_race_data_weasel_times);
    RUN_TEST(test_race_data_notfound);
}

TEST_MAIN()