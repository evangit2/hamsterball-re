/*
 * Function: FollowBall_Update
 * Address: 0x0043ECC0
 * Signature: undefined4 __fastcall FollowBall_Update(int *param_1)
 * Parameters:
 *   param_1: FollowBall* this (as int*) — the follow ball to update
 *
 * Description:
 * Per-frame update for FollowBall objects — balls that follow a predefined path.
 * Complex function that:
 *   1. Initializes Timer
 *   2. Sets Gfx_ScaleX using -+0x43C (negative scale for mirror effect)
 *   3. Computes Y position: _DAT_004D5D78 - (+0x43E × _DAT_004CF380) — interpolated height
 *   4. Calls Gfx_SetPositionAndRender with computed Y
 *   5. Calls Timer vtable+0x18 (render)
 *   6. Sets position (0, 0, 56.0 = 0x425C0000) for world-space rendering
 *   7. Restores Gfx_ScaleX using +0x43C (positive)
 *   8. Sets position from +0x435/+0x436/+0x437 (path position)
 *   9. Initializes direction at +0x438/+0x439/+0x43A to (0,0,0)
 *   10. Transforms direction by matrix (Matrix_TransformVec3)
 *   11. Calls vtable+0x58 and vtable+0x54 (sub-object update + render)
 *   12. Advances animation counter +0x43B by _DAT_004CF4DC
 *   13. If not in special mode (+0x441 == 0):
 *       a. Updates scale +0x43C using +0x43D × (speed factor from +0x43F)
 *       b. Speed factor: _DAT_004D5D68 or _DAT_004D5D70 depending on +0x43F threshold
 *   14. If +0x441 == 0: decrements +0x442 by _DAT_004D5D60
 *       a. If below threshold (_DAT_004CF310), transitions to new state
 *
 * The FollowBall follows the BallPath, with position computed from path data
 * stored at +0xC60 (set in FollowBall_Ctor).
 *
 * Struct offsets:
 *   +0x435/+0x436/+0x437: path position (X/Y/Z)
 *   +0x438/+0x439/+0x43A: direction vector
 *   +0x43B: animation counter
 *   +0x43C: scale factor
 *   +0x43D: scale velocity
 *   +0x43E: height interpolation factor
 *   +0x43F: speed parameter
 *   +0x441: special mode flag
 *   +0x442: state transition timer
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D4F64 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
