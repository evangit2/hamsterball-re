/*
 * Function: CreateBumper2
 * Address: 0x00413ce0
 *
 * Description:
 *
Arena init for Beginner Arena with 4 bumpers. MeshWorld "levels\arena-beginner",
CollisionLevel, CameraLookAt, then loops 4× calling Scene_CollectByNameFilter
"N:BUMPER%d" at Scene+0x11F8+i×0x106 (4 bumpers instead of 8 in race levels).
vtable[0x80] post-init. 1 vtable ref.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
