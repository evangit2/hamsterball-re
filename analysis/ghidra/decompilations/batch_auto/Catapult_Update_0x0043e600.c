/*
 * Function: Catapult_Update
 * Address: 0x0043E600
 * Signature: undefined4 __fastcall Catapult_Update(int *param_1)
 * Parameters:
 *   param_1: Catapult* this (as int*) — the catapult obstacle to update
 *
 * Description:
 * Per-frame update for Catapult (launch pad) obstacles. Complex function that:
 *   1. Decrements frame counter at +0x43C by +0x43D value
 *   2. Initializes Timer, sets Gfx_ScaleY/ScaleX/ScaleZ using +0x43B/+0x43A/+0x439
 *   3. Copies position from +0x436/+0x437/+0x438 to local, calls Timer vtable[2] (render)
 *   4. Calls vtable+0x58 and vtable+0x54 (update + render sub-objects)
 *   5. Iterates AthenaList at +0x43E: for each tracked object, decrements timer
 *      and removes expired entries (Gfx_SetRenderState + _free)
 *   6. Processes collision with balls on the launch pad
 *
 * The Catapult_Update manages the launch pad's animation state and tracks
 * balls currently on the pad, launching them when the timer expires.
 *
 * Struct offsets:
 *   +0x436/+0x437/+0x438: position (X/Y/Z)
 *   +0x439/+0x43A/+0x43B: rotation/scale factors
 *   +0x43C: frame counter
 *   +0x43D: decrement amount
 *   +0x43E: AthenaList (tracked objects)
 *   +0x43F: list count
 *   +0x541: list data pointer
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5AFC [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
