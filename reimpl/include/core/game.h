/*
 * Game state definitions
 */

#ifndef GAME_H
#define GAME_H

#include "physics/physics.h"

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_RACE,
    GAME_STATE_TOURNAMENT
} game_state_enum_t;

typedef struct {
    game_state_enum_t state;
    bool running;
    bool debug;
} game_state_t;

typedef struct {
    int width;
    int height;
    int fullscreen;
    int music_volume;
    int sound_volume;
    int sensitivity;
} config_t;

/* Globals declared in main.c */
extern game_state_t g_game;
extern config_t g_config;

bool config_load(config_t *cfg, const char *path);
bool config_save(config_t *cfg, const char *path);

#endif /* GAME_H */