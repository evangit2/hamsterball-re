/*
 * Function: FollowBall_Ctor
 * Address: 0x0043EBC0
 * Signature: void * __thiscall FollowBall_Ctor(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: FollowBall* — being constructed (vtable = 0x4D5D24, GameObject_sub)
 *   param_1: int — parent Scene pointer (passed to Ball_ctor2)
 *   param_2: undefined4 — stored at this+0xC60 (BallPath object pointer from Level_FindObjectByName)
 *
 * Description:
 * Constructor for FollowBall objects — special balls that follow a predefined path
 * (used in Neon Race and other levels for automated ball sequences).
 * Inherits from Ball_ctor2 (full Ball constructor), then sets up path-following.
 *
 * Steps:
 *   1. Calls Ball_ctor2(this, param_1) — full Ball construction
 *   2. Sets vtable to PTR_GameObject_sub_dtor (0x4D5D24)
 *   3. Initializes orientation matrix at +0x2AC (Vec3_Init with (0,0,1))
 *   4. Sets identity matrix
 *   5. Stores BallPath object at +0xC60 (from param_2)
 *   6. Initializes +0xC64 = 0 (path progress counter)
 *   7. Initializes position at +0x1BC/+0x1C0/+0x1C4 from Vec3_Init result
 *   8. Sets +0x1C8 = initial Y value
 *   9. Sets +0x204 = (local_10 != _DAT_004CF3C8) — boolean comparison result
 *   10. Sets identity matrix again
 *
 * The FollowBall is created by BounceBall_Update when a bounce ball reaches its
 * target — it then follows the "BallPath" found via Level_FindObjectByName.
 *
 * Struct offsets:
 *   +0x1BC/+0x1C0/+0x1C4: position (from Vec3_Init)
 *   +0x1C8: initial Y value
 *   +0x204: boolean flag (comparison result)
 *   +0x2AC: orientation matrix (Vec3_Init (0,0,1))
 *   +0xC60: BallPath object pointer
 *   +0xC64: path progress counter (0)
 *
 * Cross-references:
 *   Called from BounceBall_Update (0x4408E9) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
