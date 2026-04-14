/* jukebox.h - Jukebox XML parser matching original LoadJukebox (0x42AD80) */
#ifndef JUKEBOX_H
#define JUKEBOX_H

#include <stdint.h>

#define MAX_SONGS 64
#define MAX_SONG_NAME 128

typedef struct {
    char name[MAX_SONG_NAME];
    uint8_t hex_order;  /* Order number within .mo3 tracker module */
    int is_intro;       /* 1 = has intro, 0 = no intro version */
} jukebox_song_t;

typedef struct {
    jukebox_song_t songs[MAX_SONGS];
    int song_count;
    char mo3_path[256]; /* "music\music.mo3" */
} jukebox_t;

/* Parse jukebox.xml (original: LoadJukebox 0x42AD80) */
int jukebox_load(jukebox_t *jb, const char *xml_path);

/* Find song by name */
jukebox_song_t *jukebox_find(jukebox_t *jb, const char *name);

/* Get title screen theme */
jukebox_song_t *jukebox_title_theme(jukebox_t *jb);

/* Get race theme by race type string (e.g. "BEGINNERRACE") */
jukebox_song_t *jukebox_race_theme(jukebox_t *jb, const char *race_type);

void jukebox_free(jukebox_t *jb);

#endif