/*
 * Hamsterball Input System
 * Based on Input_Init at 0x46EE10, KeyboardDevice at 0x46E250
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <SDL2/SDL_scancode.h>

/* Game key indices - matching original +0x143 to +0x148 offsets */
typedef enum {
    INPUT_KEY_UP = 0,       /* KeyUp     (+0x143) */
    INPUT_KEY_DOWN,         /* KeyDown   (+0x144) */
    INPUT_KEY_LEFT,         /* KeyLeft   (+0x145) */
    INPUT_KEY_RIGHT,        /* KeyRight  (+0x146) */
    INPUT_KEY_ACTION1,      /* KeyAction1 (+0x147) */
    INPUT_KEY_ACTION2,      /* KeyAction2 (+0x148) */
    INPUT_KEY_COUNT
} input_key_t;

bool input_init(void);
void input_shutdown(void);
void input_update(void);

/* Key query */
bool input_is_key_pressed(input_key_t key);
void input_remap_key(input_key_t key, SDL_Scancode scancode);
SDL_Scancode input_get_scancode(input_key_t key);

/* Batch query for physics */
void input_get_all_keys(bool *up, bool *down, bool *left, bool *right,
                        bool *action1, bool *action2);

#endif /* INPUT_H */