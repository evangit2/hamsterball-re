// Hamsterball Ball struct - extracted from Ball_ctor2 (0x4039E0), Ball_ctor (0x40AFE0), Ball_Update (0x405E00)
// Total size: 0xC98 bytes (3224)

typedef float Vec3[3];

typedef struct {
    void *vtable;                  // +0x000 Ball vtable (0x4CF3A0)
    int field_04;                  // +0x004
    int collision_result;          // +0x008 from __ftol2/collision
    int string_timer;              // +0x00C countdown (200=show), frees string at 0xC28
    void *app_state;              // +0x010 App ptr (*(param_1 + 0x878))
    void *scene;                   // +0x014 Scene ptr (param_1 in ctor)
    int player_index;              // +0x018 player index (-1 = no player)
    void *render_callback;        // +0x01C vtable for render/audio callbacks
    UITimer ui_timer;            // +0x01C-0x107 UITimer (0xEC bytes)
    // gap from Timer_Init at 0x108
    int timer_108;                 // +0x108 Timer
    // gap from RenderContext_Init at 0x1B8
    int render_ctx_1b8;           // +0x1B8 RenderContext
    float render_alpha;            // +0x1C8 0x3F400000 = 0.75f
    float render_r;               // +0x1CC
    float render_g;               // +0x1D0
    float render_b;               // +0x1D4
    // gap to 0x204
    char field_204;                // +0x204 set to 1 in ctor
    char pad_205[3];              // +0x205
    // RenderContext at 0x208
    int render_ctx_208;           // +0x208 RenderContext
    float color_r;                // +0x20C RGBA (0x3F800000 = 1.0f each)
    float color_g;                // +0x210
    float color_b;                // +0x214
    float color_a;                // +0x218
    char pad_21c[4];              // +0x21C
    float tint_r;                 // +0x220 (set to 1.0f)
    float tint_g;                 // +0x224
    float tint_b;                 // +0x228
    float tint_a;                 // +0x22C
    float scale_x;               // +0x230 (set to 1.0f)
    float scale_y;               // +0x234
    float scale_z;               // +0x238
    float rgba_23c;              // +0x23C (set to 1.0f)
    float rgba_240;              // +0x240
    float rgba_244;              // +0x244
    float rgba_248;              // +0x248
    char uses_alpha;              // +0x254 bool: alpha != 1.0f
    char pad_255[3];             // +0x255
    char field_258[8];            // +0x258 padding
    char boost_hit_flag;         // +0x260 set 0 in ctor
    char pad_261[3];             // +0x261
    RumbleBoard rumble_timer;     // +0x264 RumbleBoard_InitTimer (0x14 bytes per RumbleBoard)
    char pad_278_pad[4];         // +0x278 0x3DCCCCCD = 0.1f
    float gravity_rumble_val;    // +0x27C set 0
    int field_280;                // +0x280 (0 in ctor)
    char is_falling;             // +0x281 set 1 in ctor (ball falling state)
    char pad_282[2];             // +0x282
    float radius;                 // +0x284 0x41D80000 = 27.0f
    char pad_288[4];             // +0x288 set 0
    char field_28c;               // +0x28C set 0
    char pad_28d[3];             // +0x28D
    RumbleBoard rumble_timer2;   // +0x290 pad
    int spin_timer;               // +0x2A0 (param_1[0xa8]/0xa0 in different context)
    float speed_modifier;         // +0x2A4 0x3F800000 = 1.0f (decays to 1.0)
    Vec3 some_vec;               // +0x2A8 Vec3_Init
    float max_speed_cap;          // +0x2B4 max physics value
    float accel_x;                // +0x2B8 added to pos then cleared
    float accel_y;                // +0x2BC
    float accel_z;                // +0x2C0
    char field_2cc;               // +0x2CC set 0
    char pad_2cd;                // +0x2CD
    char field_2d4;               // +0x2D4 set 0
    char field_2d5;               // +0x2D5 set 0
    int bump_timer;               // +0x2D8 set 0
    float checkpoint_x;           // +0x2DC (param_1[0xb7])
    float checkpoint_y;           // +0x2E0 (param_1[0xb8])
    float checkpoint_z;           // +0x2E4 (param_1[0xb9])
    char event_flag;              // +0x2E8 (param_1[0xba])
    char on_ramp;                 // +0x2E9 (param_1[0x2e9]) slope/ramp flag
    char pad_2ea[2];              // +0x2EA
    char spinning;                // +0x2EC (param_1[0xbb]) collision counter
    int collision_count_bc;       // +0x2F0 (param_1[0xbc])
    int collision_count_bd;       // +0x2F4 (param_1[0xbd])
    char slope_riding;            // +0x2F8 (param_1[0xbe])
    char some_flag_2f9;           // +0x2F9 (param_1[0x2f9])
    char pad_2fa[2];              // +0x2FA
    float timer_bf;               // +0x2FC (param_1[0xbf]) timer, decreased
    int timer_c0;                 // +0x300 (param_1[0xc0]) decremented, clamped
    float last_collision_x;       // +0x304 (param_1[0xc1])
    float last_collision_y;       // +0x308 (param_1[0xc2])
    float last_collision_z;       // +0x30C (param_1[0xc3])
    char collision_active;        // +0x310 (param_1[0xc4] or 0x310)
    char field_311_pad[3];        // +0x311
    char is_active;                // +0x314+0xC9=0x322... wait
    // I need to be more careful. Let me use the direct offsets.
} Ball;