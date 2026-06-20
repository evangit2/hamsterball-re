/*
 * Function: Vec3_SetScalar
 * Address: 0x004531b0
 * Signature: void __thiscall ...(void *this, undefined4 param_1)
 * Parameters:
 *   this: Vec3* | param_1: scalar value for all 3 components
 *
 * Description:
 * Sets all 3 components of Vec3 to same scalar value. Vtable 0x4CF300, x=y=z=param_1 at +4/+8/+0xC, w=1.0f at +0x10. 5+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4CF300), +0x04/+0x08/+0x0C (all=param_1), +0x10 (w=1.0)
 *
 * Cross-references:
 *   5+ calls from various UI/graphics functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
