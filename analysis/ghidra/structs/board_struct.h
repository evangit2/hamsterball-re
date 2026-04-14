// Hamsterball Board Struct Definition
// Board inherits from Gadget (base +0x000 to 0x870)
// Board vtable at 0x4D0260 (Scene_DeletingDtor)
// Board_ctor at 0x419071 (calls Gadget_ctor, sets Scene vtable)
// Total size: ~0x4368 bytes
//
// Key field groups from Board_ctor (0x419071):
//   - Gadget base (inherited): vtable, app_ptr, name, AthenaLists
//   - Scene game state: RumbleBoard timers, 13+ AthenaLists, camera, physics
//   - Ball list management: ball ptr at 0x361C, ball list at 0x362C
//   - Camera: 5 Vec3+Matrix pairs (0x3AAC-0x3AFC)
//   - Physics: gravity, scale, speed params at 0x29B0+
//   - Level config: board name, sizes, display settings

#ifndef GAME_BOARD_H
#define GAME_BOARD_H

#include <stdint.h>

typedef struct {
    // === GADGET BASE (inherited, 0x870 bytes) ===
    uint32_t vtable;              // +0x000 Scene vtable (0x4D0260)
    int32_t field_04;             // +0x004 = 0 in Board_ctor  
    int32_t field_08;             // +0x008 = 0
    float width;                  // +0x00C = 800.0f (0x44480000)
    float height;                 // +0x010 = 600.0f (0x44160000)
    uint32_t app_ptr;             // +0x014 param_1 (App*)
    uint32_t vtable_18;           // +0x018 -> 0x004CF584
    int32_t field_1c;             // +0x01C
    float field_20;               // +0x020
    float field_24;               // +0x024
    float field_28;               // +0x028
    uint8_t byte_2c;              // +0x02C
    uint32_t field_30;            // +0x030
    uint8_t list_034[0x418];      // +0x034 AthenaList - child gadgets
    uint8_t list_44c[0x420];      // +0x44C AthenaList
    
    // Pad to 0x870
    uint8_t pad_86c[0x870-0x86c]; // +0x86C  
    int32_t field_870;            // +0x870 from App->0x1DC
    uint8_t byte_874;              // +0x874 = 0
    uint32_t field_864;            // +0x864 = 0
    char* name;                    // +0x868 "Board"

    // === BOARD EXTENSION (Scene game state) ===
    uint32_t app_ptr_878;         // +0x878 param_1 (App* again)
    uint8_t pad_87c[4];            // +0x87C
    uint32_t field_880;            // +0x880 = 0
    uint8_t pad_884[4];            // +0x884
    uint8_t rumble_timer1[0x14];   // +0x884 RumbleBoard_InitTimer
    uint8_t pad_898[4];            // +0x898
    uint8_t rumble_timer2[0x14];   // +0x89C RumbleBoard_InitTimer
    uint8_t pad_8b0[4];            // +0x8B0
    uint32_t field_8ac;            // +0x8AC = 0
    uint8_t list_8b8[0x21C];       // +0x8B8 AthenaList (0x21C bytes)
    
    uint8_t pad_cd0[4];            // +0xCD0
    uint8_t list_cd4[0x418];       // +0xCD4 AthenaList
    uint8_t pad_10ec[4];           // +0x10EC
    uint8_t list_10f0[0x414];      // +0x10F0 AthenaList
    float vec3_1504[3];            // +0x1504 Vec3_Init (position)
    uint8_t list_1518[0x418];      // +0x1518 AthenaList
    uint8_t list_1930[0x418];      // +0x1930 AthenaList
    uint8_t pad_1d48[4];           // +0x1D48
    uint8_t list_1d4c[0x418];      // +0x1D4C AthenaList
    uint8_t list_2160[0x418];      // +0x2160 AthenaList
    uint8_t list_2578[0x45C];      // +0x2578 AthenaList
    
    // Physics
    float gravity_y;               // +0x29B0 = 1.0 (flag: on ground)
    uint32_t field_29b4;            // +0x29B4 = 0
    uint32_t field_29b8;           // +0x29B8 = 800 (max time limit?)
    float min_time;                 // +0x29BC = 135.0f (0x43070000)
    float max_time;                 // +0x29C0 = 1100.0f (0x44898000)
    float field_29c4;               // +0x29C4 = -1.0f (0xBF800000)
    
    // Camera lists (Vec3)
    uint32_t vec3list_29d0;         // +0x29D0 Vec3List
    uint8_t list_29d4[0x418];       // +0x29D4 AthenaList
    uint8_t list_2dec[0x418];       // +0x2DEC AthenaList
    
    // Ball management
    uint32_t field_361c;            // +0x361C Ball* ptr (second player ball)
    uint32_t field_3620;            // +0x3620
    float field_3624;               // +0x3624 = 0
    uint32_t ball_count;            // +0x3628 count of balls in list
    uint8_t list_362c[0x5D8];       // +0x362C AthenaList (ball list, size 0x5D8)
    uint8_t pad_3a44[4];            // +0x3A44
    uint8_t byte_3a48;               // +0x3A48 = 0
    
    // Ball state
    uint32_t index_list;             // +0x3A48 AthenaList ptr (IndexList)
    uint32_t field_3a50;             // +0x3A50 = 0
    uint32_t field_3a54;             // +0x3A54 = 0
    uint8_t byte_3a58;               // +0x3A58 = 1
    uint8_t byte_3a4c;               // +0x3A4C multiplayer flag
    
    // Camera matrices (4x Vec3+Matrix pairs)
    float cam_bounds1_x;            // +0x3A8C = 50.0f
    float cam_bounds1_y;            // +0x3A90 = 50.0f
    float cam_bounds1_z;            // +0x3A94 = 750.0f
    float cam_bounds2_x;            // +0x3A9C = 50.0f
    float cam_bounds2_y;            // +0x3AA0 = 600.0f (0x44098000)
    float cam_bounds2_z;            // +0x3AA4 = 750.0f (0x443B8000)
    float cam_bounds3_x;             // +0x3AA8 = 600.0f
    float cam_pos_min;              // +0x3AB0 (Vec3 Init)
    float cam_matrix1[16];          // +0x3AB0 Matrix4x4
    float cam_matrix2[16];          // +0x3AC4 Matrix4x4
    float cam_matrix3[16];          // +0x3AD8 Matrix4x4
    float cam_matrix4[16];          // +0x3AEC Matrix4x4
    
    // SceneObject sub-objects
    uint8_t obj_list_3afc[0x24];    // +0x3AFC SceneObject_EmptyListCtor

    // More AthenaLists
    uint8_t list_3204[0x428];        // +0x3204 AthenaList
    uint8_t pad_3f18[4];            // +0x3F18 = 0
    
    // Render state
    uint32_t render_target_3f20;    // +0x3F20 render target ptr
    uint8_t pad_3f40[4];            // +0x3F40
    uint32_t field_4340;             // +0x4340 = 0x3F800000 (1.0f)
    uint8_t byte_4348;               // +0x4348 = 0
    uint32_t field_4358;             // +0x4358 demo/timer flag
    uint32_t field_435c;             // +0x435C = 0x9C4 (2500) timer
    uint32_t field_4350;             // +0x4350 = 0xC3160000 (-150.0f)
    float field_4354;                // +0x4354 = 0
    uint8_t byte_4368;               // +0x4368 = 0
} Board;

#endif // GAME_BOARD_H