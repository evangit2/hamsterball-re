/*
 * Function: SceneObj_SetBounds
 * Address: 0x0040b960
 *
 * Description:
 *
Sets bounding box parameters for a SceneObject. Stores 4 float values at
SceneObject+0x14, +0x18, +0x1C, +0x20 (min/max bounds or position+size).
If the scale value at +0x10 differs from _DAT_004CF3C8 (likely 0.0 or 1.0),
sets the "has bounds" flag at +0x4C = 1; otherwise sets it to 0.
Resets the transform matrix to identity after setting bounds.

Cross-refs: 1 call — WaterRipple_Ctor.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
