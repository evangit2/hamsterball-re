/*
 * Function: AthenaList_Init
 * Address: 0x00453210
 * Signature: void * __thiscall ...(void *this, undefined4 param_1)
 * Parameters:
 *   this: AthenaList* | param_1: undefined4 (stored at +0x414)
 *
 * Description:
 * Initializes AthenaList. Sets vtable 0x4D875C. Stores param_1 at +0x414. Zeros count (+0x410), head (+4), tail (+0x40C). Zeroes 256 entries (0x400 bytes) starting at +8. +0x408=0. 4+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4D875C), +0x04 (head=0), +0x08 through +0x407 (256 entries zeroed), +0x408 (0), +0x40C (tail=0), +0x410 (count=0), +0x414 (param_1)
 *
 * Cross-references:
 *   4+ calls from MusicChannel_Ctor, Ball_ctor2, CollisionMesh_ctor, etc.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
