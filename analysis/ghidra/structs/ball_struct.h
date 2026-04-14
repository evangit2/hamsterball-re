// Hamsterball Ball Struct Definition
// Extracted from: Ball_ctor2 (0x004039E0), Ball_ctor (0x0040AFE0), Ball_Update (0x00405E00)
// Total size: 0xC98 bytes (3224 bytes)
//
// Ball is the player's ball physics object. Inherits from GameObject.
// vtable at 0x4CF3A0. Constructor sets vtable to GameObject sub-dtor at 0x4CF314.
//
// Key field groups:
//   - Identity: vtable, player_index, scene ptr, app ptr
//   - Position: pos_x/y/z (+0x164), prev_pos_x/y/z (+0x158), delta_x/y/z (+0x17C)
//   - Velocity: vel_x/y/z (+0x170), accel_x/y/z (+0x2B8)
//   - Physics: radius (+0x284), max_speed (+0x188), speed_modifier (+0x2A4)
//   - Gravity: gravity_x/y/z (+0x1A8), gravity_axis (+0x744)
//   - Collision: physics_body (+0x1A4), checkpoint (+0x2DC), bump_timer (+0x2D8)
//   - Timers: string_timer (+0x00C), spin_timer (+0x290), timer_bf (+0x2FC)
//   - Render: render_ctx_1b8 (+0x1B8), render_ctx_208 (+0x208), color RGBA (+0x20C)
//   - Rumble: rumble_timer (+0x264), rumble_timer2 (+0x290)
//   - Camera: cam_target (+0x758), cam_follow_factor (+0x764)
//   - Teleport: teleport_flag (+0xC3C), teleport_x/y/z (+0xC40)

#ifndef GAME_BALL_H
#define GAME_BALL_H

#include <stdint.h>

typedef struct {
    uint32_t vtable;          // +0x000 Ball vtable (0x4CF3A0). Set to 0x4CF314 (GameObject dtor) in ctor2
    uint32_t field_04;        // +0x004
    int32_t collision_result; // +0x008 set from __ftol2 / collision system
    int32_t string_timer;     // +0x00C countdown (200=show string). Frees string at +0xC28 when reaches 0
    uint32_t app_state;       // +0x010 App/Game state ptr (*(param_1 + 0x878) in Scene)
    uint32_t scene;           // +0x014 Scene ptr (param_1 in Ball_ctor2)
    int32_t player_index;     // +0x018 player index, -1 = AI controlled (no player)
    uint32_t render_callback; // +0x01C vtable for render/audio callbacks (+0x34 method offset)
    // --- UITimer (0xEC bytes, 0x01C to 0x107) ---
    uint8_t pad_020[0xE8];    // +0x020 UITimer subobject (0xEC bytes, ends at 0x108)
    // --- Timer (0xB0 bytes, 0x108 to 0x1B7) ---
    uint32_t timer_108;       // +0x108 Timer_Init target
    uint8_t pad_10C[0xAC];    // +0x10C Timer state area (0xB0 - 4 = 0xAC)
    // --- RenderContext #1 (0x1B8 to 0x207) ---
    uint32_t render_ctx_1b8;  // +0x1B8 RenderContext_Init target
    float render_alpha;       // +0x1C8 0x3F400000 = 0.75f
    uint32_t field_1cc;       // +0x1CC
    uint8_t pad_1D0[0x34];    // +0x1D0 padding (0x204 - 0x1D0 = 0x34)
    // --- RenderContext #2 (0x208 to 0x253) ---
    uint32_t render_ctx_208;  // +0x208 RenderContext_Init target #2
    float color_a;            // +0x20C RGBA - alpha (1.0f)
    float color_r;            // +0x210 RGBA - red   (1.0f)
    float color_g;            // +0x214 RGBA - green (1.0f)
    float color_b;            // +0x218 RGBA - blue  (1.0f)
    uint8_t pad_21C[0x14];    // +0x21C padding through tint/scale
    float tint_x;             // +0x230 tint X (1.0f init)
    float tint_y;             // +0x234 tint Y (1.0f init)
    float tint_z;             // +0x238 tint Z (1.0f init)
    float tint_w;             // +0x23C tint W (1.0f init)
    float scale_x;            // +0x240 scale X (1.0f init)
    float scale_y;            // +0x244 scale Y (1.0f init)
    float scale_z;            // +0x248 scale Z (1.0f init)
    // --- Color/Alpha ---
    uint8_t uses_alpha;       // +0x254 bool: (rgba_a != 1.0f)
    uint8_t pad_255[3];       // +0x255
    uint8_t pad_258[8];       // +0x258
    // --- Physics/State ---
    uint8_t boost_hit_flag;   // +0x260 set non-zero on boost/hit events
    uint8_t pad_261[3];       // +0x261
    // --- RumbleBoard Timer #1 (0x264, 0x14 bytes) ---
    uint8_t rumble_timer[0x14]; // +0x264 RumbleBoard_InitTimer - rumble on ball hit
    float gravity_scale;      // +0x278 0x3DCCCCCD = 0.1f
    int32_t field_27c;        // +0x27C
    int32_t field_280;        // +0x280
    uint8_t is_falling;       // +0x281 set to 1 in ctor
    uint8_t pad_282[2];       // +0x282
    float radius;             // +0x284 27.0f (0x41D80000) - ball physical radius
    int32_t field_288;        // +0x288
    uint8_t field_28c;        // +0x28C
    uint8_t pad_28d[3];       // +0x28D
    // --- RumbleBoard Timer #2 (0x290, 0x14 bytes) ---
    uint8_t rumble_timer2[0x14]; // +0x290 second RumbleBoard timer
    int32_t spin_timer;       // +0x2A4 param_1[0xa8] countdown - resets speed_modifier
    float speed_modifier;     // +0x2A8 1.0f - decays, reset on events
    float max_speed_cap;      // +0x2AC max physics calculation value
    // --- Vec3 (0x2B0, 3 floats) ---
    float vec_2b0_x;          // +0x2B0
    float vec_2b0_y;          // +0x2B4
    float vec_2b0_z;          // +0x2B8
    // --- Acceleration (accumulated, cleared each frame) ---
    float accel_x;            // +0x2BC added to position then cleared each frame
    float accel_y;            // +0x2C0
    float accel_z;            // +0x2C4
    // --- Flags ---
    uint8_t field_2cc;        // +0x2C8 set 0 in ctor
    uint8_t pad_2cd;          // +0x2C9
    uint8_t field_2d4;        // +0x2CA
    uint8_t field_2d5;        // +0x2CB
    uint8_t pad_2d6[2];       // +0x2CC
    int32_t bump_timer;       // +0x2CE set 0 in ctor
    float checkpoint_x;       // +0x2D2 param_1[0xb7] - last collision/bump position
    float checkpoint_y;       // +0x2D6 param_1[0xb8]
    float checkpoint_z;       // +0x2DA param_1[0xb9]
    uint8_t event_flag;       // +0x2DE param_1[0xba] - checkpoint hit event
    uint8_t on_ramp;          // +0x2DF param_1[0x2e9] - on ramp/slope flag
    uint8_t pad_2ea[2];       // +0x2E0
    int32_t collision_count;  // +0x2E2 param_1[0xb7] spin/collision counter
    int32_t countdown_bc;     // +0x2E6 param_1[0xbc] - decremented countdown
    int32_t countdown_bd;     // +0x2EA param_1[0xbd] - decremented countdown
    uint8_t slope_riding;     // +0x2EE param_1[0xbe] - riding slope flag
    uint8_t some_flag_2ef;    // +0x2EF param_1[0x2f9]
    uint8_t pad_2f0[2];       // +0x2F0
    float timer_bf;           // +0x2F2 param_1[0xbf] - timer decreased by constant
    int32_t timer_c0;         // +0x2F6 param_1[0xc0] - countdown, clamped >= 0
    float last_collision_x;   // +0x2FA param_1[0xc1] - position at checkpoint
    float last_collision_y;   // +0x2FE param_1[0xc2]
    float last_collision_z;   // +0x302 param_1[0xc3]
    uint8_t checkpoint_active;// +0x306 param_1[0xc4] - checkpoint active flag
    uint8_t field_307[3];     // +0x307
    // --- Position/Velocity (from Ball_Update) ---
    int32_t field_30a;        // +0x30A string ptr (freed when timer expires)
    uint8_t teleport_flag;    // +0x30E teleport triggered flag
    uint8_t pad_30f;          // +0x30F
    uint8_t pad_310[0x4C];    // +0x310 padding
    int32_t gravity_axis;     // +0x35C 0=Y-gravity, 1=X-gravity, 2=Z-gravity
    float axis_tilt_primary;  // +0x360 tilt angle for current axis
    float axis_tilt_secondary;// +0x364 secondary tilt angle
    float cam_target_x;       // +0x368 camera tracking position X
    float cam_target_y;       // +0x36C
    float cam_target_z;       // +0x370
    float cam_follow_factor;  // +0x374 camera lerp factor (1.0f init)
    uint8_t cam_active;       // +0x378 camera follow active (init 1)
    uint8_t teleport_flag2;   // +0x379 teleport-to-target flag
    uint8_t pad_37a[0x386];   // +0x37A padding
    // --- Sound/Physics handles ---
    int32_t sound_handle_3d;  // +0x700 sound 3D handle
    uint8_t pad_704[0x524];   // +0x704 padding
    // --- Display String ---
    uint32_t display_string;  // +0xC28 allocated string, freed when timer expires
    uint8_t pad_c2c[0x10];    // +0xC2C
    // --- Teleport ---
    uint8_t teleport_active;  // +0xC3C teleport active flag
    uint8_t pad_c3d[3];       // +0xC3D
    float teleport_x;         // +0xC40 teleport destination X
    float teleport_y;         // +0xC44
    float teleport_z;         // +0xC48
    uint8_t airborne;         // +0xC4C airborne flag
    uint8_t pad_c4d[3];       // +0xC4D
    float timer_c50;          // +0xC50 timer
    int32_t field_c54;        // +0xC54
    uint8_t pad_c58[4];       // +0xC58
    int32_t countdown_c5c;    // +0xC5C
    uint8_t pad_c60[0x20];    // +0xC60
    // --- Matrix/Transform ---
    uint8_t field_c80;        // +0xC80 set 0 in ctor
    uint8_t pad_c81[3];       // +0xC81
    float vec_c84_x;          // +0xC84 Vec3_Init target
    float vec_c84_y;          // +0xC88 (overlaps with matrix start)
    float vec_c84_z;          // +0xC8C
    float matrix_col0_x;      // +0xC90 4x4 transform matrix column 0
    float matrix_col0_y;      // +0xC94
    float matrix_col0_z;      // +0xC98
    float matrix_col0_w;      // +0xC9C
    float matrix_col1_x;      // +0xCA0 column 1
    float matrix_col1_y;      // +0xCA4
    float matrix_col1_z;      // +0xCA8
    float matrix_col1_w;      // +0xCAC
    float matrix_col2_x;      // +0xCB0 column 2
    float matrix_col2_y;      // +0xCB4
    float matrix_col2_z;      // +0xCB8
    float matrix_col2_w;      // +0xCBC
    float matrix_col3_x;      // +0xCC0 column 3
    float matrix_col3_y;      // +0xCC4
    float matrix_col3_z;      // +0xCC8
    float matrix_col3_w;      // +0xCCC (total size: 0xCD0? but we said 0xC98...)
} Ball;

#endif // GAME_BALL_H