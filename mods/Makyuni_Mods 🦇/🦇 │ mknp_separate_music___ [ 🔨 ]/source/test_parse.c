/* test_parse.c — native unit test for setparse.inc (the SHIPPED parser).
 * gcc -o test_parse test_parse.c && ./test_parse <setfile>
 * Keeps table names in sync with mknp_separate_music_MinGW.cpp by hand.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int nc_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

typedef struct {
    const char* name;
    const char* deffile;
    char file[128];
    int hexord;
    char id[32];
} track_entry_t;

static track_entry_t g_tracks[] = {
    {"Main Theme",                "", ""},
    {"Main Theme - No Intro",     "", ""},
    {"Cascade Race",              "", ""},
    {"Cascade Race - No Intro",   "", ""},
    {"Hamster Nation",            "", ""},
    {"Hamster Nation - No Intro", "", ""},
    {"Gerbil Groove",             "", ""},
    {"Gerbil Groove - No Intro",  "", ""},
    {"Dizzy!",                    "", ""},
    {"Dizzy! - No Intro",         "", ""},
    {"Happy Rush",                "", ""},
    {"Happy Rush - No Intro",     "", ""},
    {"Up Race",                   "", ""},
    {"Up Race - No Intro",        "", ""},
    {"Neon Theme",                "", ""},
    {"Neon Theme - No Intro",     "", ""},
    {"Fight!",                    "", ""},
    {"Fight! - No Intro",         "", ""},
    {"Ninja Hamster",             "", ""},
    {"Ninja Hamster - No Intro",  "", ""},
    {"Rodenthood",                "", ""},
    {"Rodenthood - No Intro",     "", ""},
    {"Hamster Chase",             "", ""},
    {"Hamster Chase - No Intro",  "", ""},
    {"Glass Theme",               "", ""},
    {"Glass Theme - No Intro",    "", ""},
    {"Bucky Break",               "", ""},
    {"Bucky Break - No Intro",    "", ""},
    {"Master Theme",              "", ""},
    {"Master Theme - No Intro",   "", ""},
    {"Impossible Theme",          "", ""},
    {"Impossible Theme - No Intro", "", ""},
    {"Game Over",                 "", ""},
    {"Tournament",                "", ""},
    {"Goal!",                     "", ""},
    {"High Scores",               "", ""},
    {"Loading",                   "", ""},
};
#define NUM_TRACKS (sizeof(g_tracks) / sizeof(g_tracks[0]))

#include "setparse.inc"

static int fails = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); fails++; } \
} while (0)

static track_entry_t* find(const char* name) {
    size_t i;
    for (i = 0; i < NUM_TRACKS; i++)
        if (!strcmp(g_tracks[i].name, name)) return &g_tracks[i];
    return NULL;
}

#define EXPECT(name, F, H, I) do { \
    track_entry_t* t = find(name); \
    char m[160]; \
    snprintf(m, sizeof(m), "%s mapped", name); \
    CHECK(t && t->file[0], m); \
    if (t && t->file[0]) { \
        snprintf(m, sizeof(m), "%s file=%s", name, F); \
        CHECK(!strcmp(t->file, F), m); \
        snprintf(m, sizeof(m), "%s hex=%d", name, H); \
        CHECK(t->hexord == (H), m); \
        snprintf(m, sizeof(m), "%s id=%s", name, I); \
        CHECK(!strcmp(t->id, I), m); \
    } \
} while (0)

int main(int argc, char** argv) {
    FILE* f;
    static char buf[16384];
    size_t n;
    track_entry_t* t;
    if (argc < 2) { printf("usage: test_parse <setfile>\n"); return 2; }
    f = fopen(argv[1], "rb");
    if (!f) { printf("cannot open %s\n", argv[1]); return 2; }
    n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = '\0';

    parse_set_jsonc(buf);

    /* spot checks incl hex variants + trim of "High Scores " */
    EXPECT("Hamster Nation", "level1.mo3", 0, "level1");
    EXPECT("Hamster Nation - No Intro", "level1.mo3", 0, "level1_no_intro");
    EXPECT("Cascade Race", "level2.mo3", 0, "level2");
    EXPECT("Cascade Race - No Intro", "level2.mo3", 2, "level2_no_intro");
    EXPECT("Gerbil Groove - No Intro", "level3.mo3", 2, "level3_no_intro");
    EXPECT("Dizzy!", "level4.mo3", 0, "level4");
    EXPECT("Fight!", "level8.mo3", 0, "level8");
    EXPECT("Fight! - No Intro", "level8.mo3", 1, "level8_no_intro");
    EXPECT("Ninja Hamster - No Intro", "level9.mo3", 2, "level9_no_intro");
    EXPECT("Rodenthood - No Intro", "level10.mo3", 1, "level10_no_intro");
    EXPECT("Hamster Chase - No Intro", "level11.mo3", 2, "level11_no_intro");
    EXPECT("Glass Theme", "level12.mo3", 0, "level12");
    EXPECT("Bucky Break", "level13.mo3", 0, "level13");
    EXPECT("Master Theme", "level14.mo3", 0, "level14");
    EXPECT("Impossible Theme", "level15.mo3", 0, "level15");
    EXPECT("Impossible Theme - No Intro", "level15.mo3", 0, "level15_no_intro");
    EXPECT("Loading", "loading.mo3", 0, "loader");
    EXPECT("Main Theme", "main_menu.mo3", 0, "main_menu");
    EXPECT("Main Theme - No Intro", "main_menu.mo3", 1, "main_menu_no_intro");
    EXPECT("Goal!", "goal.mo3", 0, "goal");
    EXPECT("Game Over", "game_over.mo3", 0, "game_over");
    EXPECT("High Scores", "high_scores.mo3", 0, "high_scores");
    EXPECT("Tournament", "tournament.mo3", 0, "tournament_overview");

    /* every table name present in the set file maps (incl Glass-NoIntro) */
    EXPECT("Glass Theme - No Intro", "level12.mo3", 0, "level12_no_intro");
    EXPECT("Neon Theme - No Intro", "level7.mo3", 0, "level7_no_intro");

    /* unknown names ignored: inject garbage before final ']' + reparse */
    {
        static char buf2[20000];
        char* end;
        size_t pre;
        strcpy(buf2, buf);
        end = strrchr(buf2, ']');
        if (!end) { printf("FAIL: no ] in set file\n"); return 1; }
        *end = '\0';
        pre = strlen(buf2);
        snprintf(buf2 + pre, sizeof(buf2) - pre,
            "\"zzz\", { \"name\": \"No Such Song\", \"file\": \"x.mo3\", \"hex\": \"05\" },\n"
            "\"dup\", { \"name\": \"Goal!\", \"file\": \"goal2.mo3\", \"hex\": \"03\" },\n"
            "\"bad\", { \"name\": \"Goal!\" },\n]");
        parse_set_jsonc(buf2);
        t = find("Goal!");
        CHECK(t && !t->file[0] && t->hexord == 0, "Goal! cleared by entry without file");
    }

    if (!fails) printf("PARSE TEST PASS\n");
    else printf("PARSE TEST %d FAILURES\n", fails);
    return fails ? 1 : 0;
}
