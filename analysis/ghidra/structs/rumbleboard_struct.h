// Hamsterball RumbleBoard Struct Definition
// RumbleBoard is embedded in Scene (+0x221, +0x226) and Board (+0x884, +0x898)
// Size: 0x14 bytes (20 bytes) - from RumbleBoard_InitTimer calls
//
// Used for haptic feedback timing when the ball hits surfaces.
// The game calls RumbleBoard_InitTimer at construction and 
// RumbleBoard_CleanupTimer at destruction.

#ifndef GAME_RUMBLEBOARD_H
#define GAME_RUMBLEBOARD_H

#include <stdint.h>

typedef struct {
    uint32_t timer_active;        // +0x00 whether rumble timer is active
    uint32_t timer_id;            // +0x04 timer identifier
    float intensity;              // +0x08 rumble intensity
    float duration;               // +0x0C rumble duration
    uint32_t callback_ptr;        // +0x10 callback function pointer
} RumbleBoard;

// Total size: 0x14 (20 bytes)

#endif // GAME_RUMBLEBOARD_H