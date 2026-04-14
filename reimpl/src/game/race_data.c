/* race_data.c - RaceData.xml parser */
#include "game/race_data.h"
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

static float get_float_tag(char *block, const char *tag) {
    char open[128], close[128];
    snprintf(open, sizeof(open), "<%s>", tag);
    snprintf(close, sizeof(close), "</%s>", tag);
    char *s = strstr(block, open);
    if (!s) return 0.0f;
    s += strlen(open);
    return (float)atof(s);
}

static int get_int_tag(char *block, const char *tag) {
    return (int)get_float_tag(block, tag);
}

/* Known race type tags in order */
static const char *RACE_TAGS[] = {
    "BEGINNERRACE", "CASCADERACE", "INTERMEDIATERACE", "DIZZYRACE",
    "TOWERRACE", "UPRACE", "NEONRACE", "EXPERTRACE",
    "ODDRACE", "TOOBRACE", "WOBBLYRACE", "GLASSRACE",
    "SKYRACE", "MASTERRACE", "IMPOSSIBLERACE", NULL
};

int race_data_load(race_data_set_t *rds, const char *xml_path) {
    memset(rds, 0, sizeof(*rds));
    
    char *text = read_file_text(xml_path);
    if (!text) return -1;
    
    for (int i = 0; RACE_TAGS[i] && rds->race_count < MAX_RACE_TYPES; i++) {
        char open[128], close[128];
        snprintf(open, sizeof(open), "<%s>", RACE_TAGS[i]);
        snprintf(close, sizeof(close), "</%s>", RACE_TAGS[i]);
        
        char *s = strstr(text, open);
        if (!s) continue;
        char *e = strstr(s, close);
        if (!e) continue;
        
        /* Extract block between tags */
        char *block_start = s + strlen(open);
        size_t block_len = e - block_start;
        char *block = malloc(block_len + 1);
        memcpy(block, block_start, block_len);
        block[block_len] = '\0';
        
        race_data_t *rd = &rds->races[rds->race_count++];
        strncpy(rd->name, RACE_TAGS[i], sizeof(rd->name) - 1);
        rd->time_limit = get_int_tag(block, "TIME");
        rd->par_time = get_float_tag(block, "PAR");
        rd->weasel_time = get_float_tag(block, "WEASEL");
        rd->gold_time = get_float_tag(block, "GOLD");
        rd->silver_time = get_float_tag(block, "SILVER");
        rd->bronze_time = get_float_tag(block, "BRONZE");
        rd->cam_param = get_float_tag(block, "CAM");
        
        free(block);
    }
    
    free(text);
    return rds->race_count > 0 ? 0 : -1;
}

race_data_t *race_data_find(race_data_set_t *rds, const char *name) {
    for (int i = 0; i < rds->race_count; i++) {
        if (strcmp(rds->races[i].name, name) == 0) return &rds->races[i];
    }
    return NULL;
}

void race_data_free(race_data_set_t *rds) {
    memset(rds, 0, sizeof(*rds));
}