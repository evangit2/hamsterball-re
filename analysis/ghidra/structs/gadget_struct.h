// Hamsterball Gadget/SceneObject Base Struct
// Gadget is the base class for all scene objects.
// vtable at 0x4D9170 (SceneObject_ScalarDtor).
// Base size: 0x870 bytes (from Gadget_ctor at 0x4690F0)
//
// Inheritance: Gadget -> Board -> RumbleBoard
// Scene inherits from Gadget (uses Scene vtable 0x4D0260 instead)
//
// From Gadget_ctor (0x4690F0):
//   +0x000 vtable -> 0x4D9170
//   +0x004 field_4 = 0
//   +0x008 field_8 = 0
//   +0x00C field_c = 0
//   +0x010 field_10 = 0
//   +0x014 param_1 (App ptr)
//   +0x018 second vtable ptr -> 0x004CF584
//   +0x01C field_1c = 0
//   +0x020 field_20 = 0
//   +0x024 field_24 = 0
//   +0x028 field_28 = 0
//   +0x02C byte_2c = 0
//   +0x030 field_30 = 0
//   +0x034 AthenaList (list of child Gadgets)
//   +0x044 AthenaList (another list)
//   +0x86C byte_86c = 0
//   +0x86D byte_86d = 0
//   +0x86E byte_86e = 0
//   +0x86F byte_86f = 0
//   +0x870 field_870 = 0
//   +0x874 byte_874 = 0
//   +0x864 field_864 = 0
//   +0x868 name = "Generic Gadget"
//   +0x870 field_870 = App ptr

#ifndef GAME_GADGET_H
#define GAME_GADGET_H

#include <stdint.h>

typedef struct {
    uint32_t vtable;              // +0x000 SceneObject_ScalarDtor vtable (0x4D9170)
    int32_t field_04;              // +0x004
    int32_t field_08;              // +0x008
    int32_t field_0c;              // +0x00C
    uint32_t field_10;             // +0x010
    uint32_t app_ptr;              // +0x014 param_1 from ctor (App*)
    uint32_t vtable_18;            // +0x018 -> 0x004CF584
    int32_t field_1c;              // +0x01C
    int32_t field_20;              // +0x020
    int32_t field_24;              // +0x024
    int32_t field_28;              // +0x028
    uint8_t byte_2c;                // +0x02C
    int32_t field_30;              // +0x030
    // AthenaList +0x034 (0x418 bytes in Board)
    uint8_t list_034[0x418];       // +0x034 AthenaList_Init - child gadgets list
    // Second AthenaList at +0x44C
    uint8_t list_44c[0x420];       // +0x44C AthenaList_Init
    uint8_t pad_86c[4];            // +0x86C
    int32_t field_870;             // +0x870 from *(param_1 + 0x1dc)
    uint8_t byte_874;              // +0x874
    int32_t field_864;             // +0x864
    char* name;                    // +0x868 "Generic Gadget" / "Board" / etc.
} Gadget;

#endif // GAME_GADGET_H
