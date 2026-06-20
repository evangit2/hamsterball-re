/*
 * Function: Vec3_Init3Comp
 * Address: 0x00453180
 * Signature: void __thiscall ...(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3)
 * Parameters:
 *   this: Vec3* | param_1-3: x/y/z components
 *
 * Description:
 * Initializes Vec3 with 3 components + w=1.0. Sets vtable 0x4CF300, stores x at +4, y at +8, z at +0xC, w=1.0f at +0x10. 4+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4CF300), +0x04 (x), +0x08 (y), +0x0C (z), +0x10 (w=1.0)
 *
 * Cross-references:
 *   4+ calls from Level_InitMeshes4
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
