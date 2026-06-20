/*
 * Function: Vec3_InitZero
 * Address: 0x004531e0
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Vec3* this — being initialized to zero
 *
 * Description:
 * Initializes Vec3 to zero with large default W. Sets vtable 0x4CF300, x=y=z=0, w=255.0f (0x437F0000). 4+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4CF300), +0x04/+0x08/+0x0C (all=0), +0x10 (w=255.0)
 *
 * Cross-references:
 *   4+ calls from Ball_ctor2, UIListItem_ctor, Ball_ctor, SceneObject_BaseInit
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
