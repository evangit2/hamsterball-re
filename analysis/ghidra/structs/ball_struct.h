// Hamsterball Ball Struct - Ghidra Applied Definition
// Extracted from: Ball_ctor2 (0x4039E0), Ball_Update (0x405E00), Ball_ctor (0x40AFE0), Ball_dtor2 (0x401CC0)
// Total size: 0xC98 bytes (3224 bytes) in game, 3148 bytes in Ghidra struct (key named fields)
// Vtable: 0x4CF3A0 (9 method pointers)
// Constructor sets vtable to 0x4CF314 (GameObject_sub2_dtor)
//
// Ghidra struct "Ball" created with 38 named fields.
// NOTE: Ghidra `this` parameter retyping is broken (ECX register vars can't be retyped via API).
// Workaround: plate comments on key functions document all field offsets.
// To fully apply: retype `this` parameter manually in Ghidra GUI (right-click -> Retype Variable)

#ifndef GAME_BALL_H
#define GAME_BALL_H

#include <stdint.h>

// Verified field offsets from Ball_ctor2 decompilation
typedef struct {
    // === Identity (0x00-0x1C) ===
    uint32_t vtable;              // +0x000 Ball vtable (0x4CF3A0). Ctor sets to 0x4CF314
    uint32_t field_04;            // +0x004 set 0
    int32_t collision_result;     // +0x008 from __ftol2 / collision system
    int32_t string_timer;        // +0x00C countdown (200=show). Frees string at +0xC28 when 0
    uint32_t app_state;           // +0x010 *(param_1 + 0x878) from Scene
    uint32_t scene;               // +0x014 Scene ptr (param_1 in ctor2)
    int32_t player_index;        // +0x018 -1 = AI controlled, 0+ = player
    uint32_t render_callback;    // +0x01C vtable for render/audio callbacks

    // === UITimer (0x1C to 0x107) ===
    uint8_t uitimer[0xEC];        // +0x01C UITimer sub-object (0xEC bytes)

    // === Timer (0x108 to 0x1A7) ===
    uint32_t timer;               // +0x108 Timer_Init target
    uint8_t timer_pad[0x9C];      // +0x10C timer state area

    // === Position/Velocity (from Ball_Update 0x405E00) ===
    float accumulated_time;       // +0x150 accumulated delta time
    uint8_t pad_154[4];           // +0x154 rng_seed from RNG_Rand
    float prev_pos_x;             // +0x158 previous frame position X
    float prev_pos_y;             // +0x15C
    float prev_pos_z;             // +0x160
    float pos_x;                  // +0x164 current position X (set 0 in ctor)
    float pos_y;                  // +0x168
    float pos_z;                  // +0x16C
    float vel_x;                  // +0x170 velocity (set 0 in ctor)
    float vel_y;                  // +0x174
    float vel_z;                  // +0x178

    // === Physics ===
    float max_speed;              // +0x188 0x459C4000 = 5000.0f
    float speed_scale;            // +0x18C 0x3F800000 = 1.0f
    uint8_t field_19c;           // +0x19C set 0 in ctor
    uint8_t pad_19d[3];          // +0x19D
    float max_speed_cap;          // +0x1A0 0x3F800000 = 1.0f
    uint8_t pad_1a4[4];          // +0x1A4 collision_mesh ptr (CollisionMesh_ctor)
    uint8_t pad_1a8[4];          // +0x1A8 gravity data (3 floats: x=0, y=1.0f, z=0)

    // === Render Context #1 (0x1B8) ===
    uint32_t render_ctx_1;        // +0x1B8 RenderContext_Init target
    float render_field_1bc;       // +0x1BC 0x3E800000
    float render_field_1c0;       // +0x1C0 0x3E800000
    float render_field_1c4;       // +0x1C4 0x3E800000
    float render_alpha;          // +0x1C8 0x3F400000 = 0.75f
    uint8_t pad_1cc[0x40];       // +0x1CC

    // === Render Context #2 (0x208) ===
    uint32_t render_ctx_2;        // +0x208 RenderContext_Init target
    float color_a;               // +0x20C RGBA alpha (1.0f = 0x3F800000)
    float color_r;               // +0x210 RGBA red   (1.0f)
    float color_g;               // +0x214 RGBA green (1.0f)
    float color_b;               // +0x218 RGBA blue  (1.0f)
    uint8_t pad_21c[0x38];       // +0x21C

    // === Tint/Scale/Alpha ===
    float tint_x;                // +0x23C 1.0f
    float tint_y;                // +0x240 1.0f
    float tint_z;                // +0x244 1.0f
    float tint_w;                // +0x248 1.0f
    uint8_t uses_alpha;          // +0x254 bool: color_a != 1.0f
    uint8_t pad_255[0xB];       // +0x255

    // === Physics/State ===
    uint8_t boost_flag;           // +0x260 set 0 in ctor
    uint8_t pad_261[3];          // +0x261

    // === RumbleBoard Timer #1 (0x264, 0x14 bytes) ===
    uint8_t rumble_timer1[0x14]; // +0x264 RumbleBoard_InitTimer

    // === Gravity/Falling ===
    float gravity_scale;          // +0x278 0x3DCCCCCD = 0.1f
    uint32_t field_27c;           // +0x27C = 0
    uint8_t field_280;            // +0x280 = 0
    uint8_t is_falling;           // +0x281 set to 1 in ctor
    uint8_t pad_282[2];          // +0x282
    float radius;                 // +0x284 0x41D80000 = 27.0f
    uint32_t field_288;           // +0x288 = 0
    uint8_t field_28c;            // +0x28C = 0
    uint8_t pad_28d[3];          // +0x28D

    // === RumbleBoard Timer #2 + spin (0x290) ===
    uint8_t rumble_timer2[0x14]; // +0x290 second RumbleBoard timer
    int32_t field_2a4;           // +0x2A4 0x40A00000 = 5.0f
    uint8_t pad_2a8[0x14];       // +0x2A8 Vec3_Init target

    // === Acceleration (cleared each frame) ===
    uint8_t pad_2bc[4];          // +0x2BC acceleration X (from Ball_Update)
    uint8_t pad_2c0[4];          // +0x2C0 accel Y
    uint8_t pad_2c4[4];          // +0x2C4 accel Z
    uint8_t field_2cc;            // +0x2CC set 0
    uint8_t field_2d4;            // +0x2D4 set 0
    uint8_t field_2d5;            // +0x2D5 set 0
    uint8_t pad_2d6[2];          // +0x2D6
    uint32_t field_2d8;           // +0x2D8 set 0

    // === Checkpoint/Collision ===
    float checkpoint_x;           // +0x2DC last collision/bump position
    float checkpoint_y;           // +0x2E0
    float checkpoint_z;           // +0x2E4
    uint8_t event_flag;           // +0x2E8 checkpoint hit event
    uint8_t on_ramp;              // +0x2E9 on slope/ramp flag
    uint8_t field_2f9;            // +0x2F9
    uint8_t field_2f8;            // +0x2F8
    uint32_t field_2ec;           // +0x2EC collision counter
    uint32_t field_2f0;           // +0x2F0
    uint32_t field_2f4;           // +0x2F4
    float timer_bf;               // +0x2FC 0x3F800000 = 1.0f
    uint32_t field_300;           // +0x300
    uint8_t field_310;            // +0x310 = 1

    // === Camera (0x758+) ===
    uint8_t pad_31c[0x768-0x31c]; // +0x31C
    uint8_t cam_active;           // +0x768 = 1 (camera follow on)
    uint8_t pad_769;             // +0x769 = 0
    float cam_follow_factor;      // +0x764 = 1.0f cam lerp factor
    uint8_t pad_770[0x7C8-0x76A]; // +0x76A
    float matrix_1[16];           // +0x7C8 4x4 transform matrix (cleared to 0)
    float matrix_2[16];          // +0x808 4x4 transform matrix

    // === Sound ===
    int32_t sound_3d_handle;      // +0x700 3D sound handle

    // === Display ===
    uint32_t display_string;      // +0xC28 allocated string, freed when timer expires
    uint8_t pad_c2c;             // +0xC2C
    uint8_t pad_c38[4];          // +0xC38 = -1
    uint8_t teleport_active;     // +0xC3C teleport flag
    uint8_t pad_c3d[3];          // +0xC3D
    float teleport_x;            // +0xC40 teleport destination
    float teleport_y;            // +0xC44
    float teleport_z;            // +0xC48
    uint8_t airborne;            // +0xC4C airborne flag

    // === Matrix/Transform ===
    uint8_t pad_c50[0xC88-0xC50]; // +0xC50
    float matrix_4x4[16];        // +0xC88 4x4 world transform matrix
} Ball;

#endif // GAME_BALL_H