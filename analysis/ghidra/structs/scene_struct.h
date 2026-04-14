// Hamsterball Scene Struct Definition
// From Scene_dtor (0x419770), Board_ctor (0x419020), Scene vtable at 0x4D0260
// Scene inherits from Gadget (0x870 base), total size ~0x1000+
//
// Scene is the main game state container. It holds:
//   - Ball objects (player balls)
//   - Board objects (level geometry)
//   - Effect/sound/particle systems
//   - Camera and render state
//
// Cleanup order from Scene_dtor:
//   1. +0xd87 vtable (app state callback)
//   2. AthenaList at +0x335 (Ball list, 0x438 array)
//   3. AthenaList at +0x43b (effect list, 0x53e array)
//   4. AthenaList at +0x22e (object list, 0x331 array)
//   5. +0x22c, +0x22b, +0x21f, +0xe92 vtable dtors
//   6. AthenaList at +0xa75 (0xb78 array)
//   7. AthenaList at +0xc81 (0xd84 array)
//   8. Vec3List at +0x858, +0x64c, +0x335, +0x43b
//   9. +0xfc6 vtable dtor
//   10. SceneObject_BaseDtor at +0xebf
//   11. Vec3 arrays at +0xeab (4 Vec3s)
//   12. Vec3List at +0xd8b, +0xb7b, +0x95e, +0x858, +0x752, +0x64c, +0x546, +0x43b, +0x335, +0x22e
//   13. RumbleBoard_CleanupTimer at +0x226, +0x221
//   14. SceneObject_dtor

#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include <stdint.h>

// RumbleBoard embedded at +0x221 (from RumbleBoard_CleanupTimer calls)
typedef struct {
    uint8_t rumble_data[0x20];  // +0x221 RumbleBoard data (20 bytes)
} RumbleBoard_embedded;

// Scene inherits Gadget base (+0x000 to +0x870)
// Gadget base: vtable, app_ptr, name, AthenaList children
// Scene adds game state on top (+0x870 onwards)

typedef struct {
    // === GADGET BASE (inherited, 0x870 bytes) ===
    uint32_t vtable;              // +0x000 Scene vtable (0x4D0260)
    int32_t field_04;             // +0x004
    int32_t field_08;             // +0x008
    int32_t field_0c;             // +0x00C
    uint32_t field_10;            // +0x010
    uint32_t app_ptr;             // +0x014 param_1 (App*)
    uint32_t vtable_18;           // +0x018 -> 0x004CF584
    int32_t field_1c;             // +0x01C
    int32_t field_20;             // +0x020
    int32_t field_24;             // +0x024
    int32_t field_28;             // +0x028
    uint8_t byte_2c;              // +0x02C
    int32_t field_30;             // +0x030
    uint8_t list_034[0x418];      // +0x034 AthenaList - child gadgets list
    uint8_t list_44c[0x420];      // +0x44C AthenaList - another list
    uint8_t pad_86c[4];           // +0x86C
    int32_t field_870;            // +0x870
    uint8_t byte_874;             // +0x874
    int32_t field_864;             // +0x864
    char* name;                   // +0x868 "Generic Gadget" / "Board"
    
    // === SCENE EXTENSION ===
    // RumbleBoard sub-object (from RumbleBoard_CleanupTimer at +0x226, +0x221)
    uint8_t pad_878[0x221-0x878]; // +0x878 padding to RumbleBoard
    uint8_t rumble_board[0x20];   // +0x221 RumbleBoard timer data
    
    // AthenaList at +0x22e (element size 0x334, count at +0x22f)
    uint8_t list_22e[0x100];     // +0x241 AthenaList - main object list
    
    // Vec3List cleanup targets (from dtor)
    uint8_t pad_33d[0x335-0x33d]; // +0x241
    // +0x335 Vec3List (element 0x100, count at +0x336) - Ball list
    uint8_t ball_list[0x100];    // +0x335
    uint8_t pad_435[0x43b-0x435];// +0x435
    // +0x43b Vec3List - effect list
    uint8_t effect_list[0x100];  // +0x43B
    
    uint8_t pad_53b[0x546-0x53b];// +0x53B
    uint8_t list_546[0x100];     // +0x546 Vec3List
    uint8_t pad_646[0x64c-0x646];// +0x646
    uint8_t list_64c[0x100];     // +0x64C Vec3List
    uint8_t pad_74c[0x752-0x74c];// +0x74C
    uint8_t list_752[0x100];     // +0x752 Vec3List
    uint8_t pad_852[0x858-0x852];// +0x852
    uint8_t list_858[0x100];     // +0x858 Vec3List
    
    // vtable cleanup at +0x21f, +0x22b, +0x22c, +0xe92
    uint8_t pad_95e[0x95e-0x958];// +0x958
    uint8_t list_95e[0x100];     // +0x95E Vec3List
    uint8_t pad_a5e[0xb78-0xa5e];// +0xA5E
    // +0xa75 AthenaList (element size 0x100, count at +0xa76, array at +0xb78)
    uint8_t list_a75[0x100];     // +0xA75
    uint8_t pad_b7b[0xb7b-0xb75];// +0xB75
    uint8_t veclist_b7b[0x100];   // +0xB7B Vec3List
    uint8_t pad_c7b[0xc81-0xc7b];// +0xC7B
    // +0xc81 AthenaList (element size 0x100, count at +0xc82, array at +0xd84)
    uint8_t list_c81[0x100];     // +0xC81
    uint8_t pad_d81[0xd8b-0xd81];// +0xD81
    uint8_t veclist_d8b[0x100];  // +0xD8B Vec3List
    
    uint8_t pad_e7b[0xeab-0xe7b];// +0xE7B
    // +0xeab Vec3[4] array (4 * 0x14 = 0x50 bytes)
    float vec4_0_x; float vec4_0_y; float vec4_0_z; float vec4_0_w; // +0xEAB
    float vec4_1_x; float vec4_1_y; float vec4_1_z; float vec4_1_w; // +0xEBB
    float vec4_2_x; float vec4_2_y; float vec4_2_z; float vec4_2_w; // +0xECB
    float vec4_3_x; float vec4_3_y; float vec4_3_z; float vec4_3_w; // +0xEDB
    
    // SceneObject base dtor at +0xebf (sub-object embedded in Scene)
    uint8_t sceneobject_base[0x30]; // +0xEEB SceneObject_Base (from SceneObject_BaseDtor)
    
    uint8_t pad_f1b[0xfc6-0xf1b];  // +0xF1B
    uint32_t vtable_fc6;           // +0xFC6 vtable dtor (Scene_DeletingDtor)
    uint8_t pad_fca[0xfc8-0xfca]; // +0xFCA
    
    // +0xfc8 eh_vector destructor (2 elements of size 0x10)
    uint8_t eh_vec_fc8[0x20];     // +0xFC8
} Scene;

#endif // GAME_SCENE_H
