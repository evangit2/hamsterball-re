/*
 * Hamsterball Input System
 * 
 * Based on reverse engineering of Input_Init at 0x46EE10
 * KeyboardDevice_ctor at 0x46E250
 * 6 game keys: Up/Down/Left/Right/Action1/Action2
 * Key mappings stored in KeyboardDevice at offsets +0x143 to +0x148
 */

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "input/input.h"

static SDL_Scancode key_bindings[6] = {
    SDL_SCANCODE_UP,        /* KeyUp    (+0x143) */
    SDL_SCANCODE_DOWN,      /* KeyDown  (+0x144) */
    SDL_SCANCODE_LEFT,      /* KeyLeft  (+0x145) */
    SDL_SCANCODE_RIGHT,     /* KeyRight (+0x146) */
    SDL_SCANCODE_SPACE,     /* KeyAction1 (+0x147) */
    SDL_SCANCODE_LSHIFT,    /* KeyAction2 (+0x148) */
};

static const Uint8 *key_state = NULL;

bool input_init(void) {
    key_state = SDL_GetKeyboardState(NULL);
    if (!key_state) {
        fprintf(stderr, "[INPUT] Failed to get keyboard state\n");
        return false;
    }
    printf("[INPUT] Initialized (DirectInput8 equivalent)\n");
    return true;
}

void input_shutdown(void) {
    printf("[INPUT] Shutdown\n");
}

void input_update(void) {
    /* SDL keyboard state is updated by SDL_PumpEvents() */
    /* which is called in SDL_PollEvent() */
}

/* Remap a game key (like original Remap Keyboard at 0x4D622C) */
void input_remap_key(input_key_t key, SDL_Scancode scancode) {
    if (key >= 0 && key < INPUT_KEY_COUNT) {
        key_bindings[key] = scancode;
    }
}

/* Check if a game key is pressed */
bool input_is_key_pressed(input_key_t key) {
    if (key >= 0 && key < INPUT_KEY_COUNT && key_state) {
        return key_state[key_bindings[key]] != 0;
    }
    return false;
}

/* Get scancode for a game key */
SDL_Scancode input_get_scancode(input_key_t key) {
    if (key >= 0 && key < INPUT_KEY_COUNT) {
        return key_bindings[key];
    }
    return SDL_SCANCODE_UNKNOWN;
}

/* Get all key states at once (for physics) */
void input_get_all_keys(bool *up, bool *down, bool *left, bool *right,
                        bool *action1, bool *action2) {
    if (key_state) {
        *up      = key_state[key_bindings[INPUT_KEY_UP]] != 0;
        *down    = key_state[key_bindings[INPUT_KEY_DOWN]] != 0;
        *left    = key_state[key_bindings[INPUT_KEY_LEFT]] != 0;
        *right   = key_state[key_bindings[INPUT_KEY_RIGHT]] != 0;
        *action1 = key_state[key_bindings[INPUT_KEY_ACTION1]] != 0;
        *action2 = key_state[key_bindings[INPUT_KEY_ACTION2]] != 0;
    } else {
        *up = *down = *left = *right = *action1 = *action2 = false;
    }
}