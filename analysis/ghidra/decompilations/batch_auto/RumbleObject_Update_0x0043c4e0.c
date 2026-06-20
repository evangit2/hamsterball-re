/*
 * Function: RumbleObject_Update
 * Address: 0x0043C4E0
 * Signature: undefined4 __fastcall RumbleObject_Update(int *param_1)
 * Parameters:
 *   param_1: RumbleObject* this (as int*) — the rumble object to update
 *
 * Description:
 * Per-frame update for RumbleObject (Blockdawg) obstacles. Complex function:
 *   1. Generates random frame offset: RNG_Rand(10) × constant + base
 *   2. If not in special mode (+0x454 == 0):
 *      a. Advances along path using Path_GetPosition — follows movement path
 *      b. Calculates direction vector from current to next path position
 *      c. Normalizes direction with Vec3_NormalizeAndScale(1.0)
 *      d. Updates stored position at +0x436/+0x438
 *   3. If in special mode:
 *      a. Uses alternative movement logic (likely chase/attack behavior)
 *
 * The Blockdawg follows a predefined path (stored at +0x10F0 from Blockdawg_ctor)
 * through the level, moving toward the ball or along patrol routes.
 *
 * Struct offsets:
 *   +0x436/+0x437/+0x438: current position (X/Y/Z)
 *   +0x439/+0x43A/+0x43B: direction vector
 *   +0x43C: path data pointer
 *   +0x44E: path segment index
 *   +0x44F: movement speed multiplier
 *   +0x450: random frame offset
 *   +0x454: special mode flag (0 = path follow, 1 = chase)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5664 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
