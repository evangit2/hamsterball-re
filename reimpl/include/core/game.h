/* game.h - Core game types and state */
#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

/* Game states matching original app flow */
typedef enum {
    GAME_STATE_LOADING = 0,
    GAME_STATE_MENU,
    GAME_STATE_COUNTDOWN,
    GAME_STATE_RACING,
    GAME_STATE_RESULTS,
} game_state_t;

/* Vec3 - matches original Vec3_Init (0x4531E0): x,y,z + w=255.0 */
typedef struct { float x, y, z; } vec3_t;

/* Ball state - matches Ball object at 0xC98 bytes, key fields */
typedef struct {
    vec3_t position;      /* +0x164 */
    vec3_t velocity;      /* +0x170 */
    vec3_t acceleration;  /* +0x17C */
    float radius;         /* +0x284 = 35.0f (0x420C0000) */
    float health;         /* 1.0 = alive, 0.0 = dead */
    bool on_ground;
    bool in_tar;          /* N:TARPIT */
    bool dizzy;           /* N:NOCONTROL / dizzy state */
    bool in_water;        /* N:WATER */
    float angular_velocity; /* Rolling visual speed */
    int freeze_counter;   /* +0x808: skip force if > 0 */
} ball_t;

#endif