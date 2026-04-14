/* jukebox.c - Jukebox XML parser matching original LoadJukebox (0x42AD80) */
#include "audio/jukebox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *read_file_text(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

/* Find tag content within a bounded region of text */
static int extract_tag(char *start, char *end, const char *tag, char *out, size_t out_size) {
    char open[128], close[128];
    snprintf(open, sizeof(open), "<%s>", tag);
    snprintf(close, sizeof(close), "</%s>", tag);
    
    /* Search only within [start, end) */
    size_t region_len = end - start;
    char *s = NULL;
    for (char *p = start; p + strlen(open) <= end; p++) {
        if (memcmp(p, open, strlen(open)) == 0) {
            s = p + strlen(open);
            break;
        }
    }
    if (!s) return -1;
    
    /* Skip leading whitespace */
    while (s < end && isspace((unsigned char)*s)) s++;
    
    /* Find closing tag within region */
    char *e = NULL;
    for (char *p = s; p + strlen(close) <= end; p++) {
        if (memcmp(p, close, strlen(close)) == 0) {
            e = p;
            break;
        }
    }
    if (!e) return -1;
    
    /* Trim trailing whitespace */
    while (e > s && isspace((unsigned char)*(e - 1))) e--;
    
    size_t len = e - s;
    if (len >= out_size) len = out_size - 1;
    memcpy(out, s, len);
    out[len] = '\0';
    return 0;
}

int jukebox_load(jukebox_t *jb, const char *xml_path) {
    memset(jb, 0, sizeof(*jb));
    
    char *text = read_file_text(xml_path);
    if (!text) return -1;
    
    char *pos = text;
    while (pos && *pos && jb->song_count < MAX_SONGS) {
        /* Find next <SONG> */
        char *song_start = strstr(pos, "<SONG>");
        if (!song_start) break;
        
        /* Find </SONG> */
        char *song_end = strstr(song_start, "</SONG>");
        if (!song_end) break;
        
        char name[256] = "", hex[32] = "";
        
        if (extract_tag(song_start, song_end, "NAME", name, sizeof(name)) == 0 &&
            extract_tag(song_start, song_end, "HEX", hex, sizeof(hex)) == 0) {
            
            jukebox_song_t *s = &jb->songs[jb->song_count++];
            strncpy(s->name, name, sizeof(s->name) - 1);
            s->hex_order = (uint8_t)strtol(hex, NULL, 16);
            s->is_intro = (strstr(name, "No Intro") == NULL);
        }
        
        pos = song_end + 7; /* past </SONG> */
    }
    
    free(text);
    strncpy(jb->mo3_path, "music\\music.mo3", sizeof(jb->mo3_path) - 1);
    return jb->song_count > 0 ? 0 : -1;
}

jukebox_song_t *jukebox_find(jukebox_t *jb, const char *name) {
    for (int i = 0; i < jb->song_count; i++) {
        if (strstr(jb->songs[i].name, name)) return &jb->songs[i];
    }
    return NULL;
}

jukebox_song_t *jukebox_title_theme(jukebox_t *jb) {
    return jukebox_find(jb, "Main Theme");
}

jukebox_song_t *jukebox_race_theme(jukebox_t *jb, const char *race_type) {
    (void)jb; (void)race_type;
    return NULL;
}

void jukebox_free(jukebox_t *jb) {
    memset(jb, 0, sizeof(*jb));
}