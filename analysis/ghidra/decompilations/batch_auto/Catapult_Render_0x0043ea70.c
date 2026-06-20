/*
 * Function: Catapult_Render
 * Address: 0x0043EA70
 * Signature: undefined4 __fastcall Catapult_Render(int *param_1)
 * Parameters:
 *   param_1: Catapult* this (as int*) — the catapult to render
 *
 * Description:
 * Render function for Catapult obstacles. Steps:
 *   1. Initializes Timer
 *   2. Computes Z-scale using Wave_Sin with angle at +0x53F — creates oscillating
 *      animation (launch pad bobs/rotates)
 *   3. Copies position from +0x436/+0x437/+0x438, calls Timer vtable[2] (render setup)
 *   4. Advances animation: +0x53F += +0x540 (rotation speed)
 *   5. Calls vtable+0x58 and vtable+0x54 (sub-object update + render)
 *   6. Iterates AthenaList at +0x439: decrements per-entry timers, removes expired
 *      entries via Gfx_SetRenderState + _free
 *   7. Cleans up Timer
 *
 * The catapult oscillates using a sine wave animation while rendering and
 * managing its tracked objects (balls waiting to be launched).
 *
 * Struct offsets:
 *   +0x436/+0x437/+0x438: position (X/Y/Z)
 *   +0x439: AthenaList (tracked objects)
 *   +0x43A: list count
 *   +0x53C: list data pointer
 *   +0x53F: animation angle (advanced each frame)
 *   +0x540: rotation speed (added to angle each frame)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5B5C [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
