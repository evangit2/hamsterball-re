/*
 * Function: Gear_Level_ctor
 * Address: 0x0043A150
 * Signature: void * __thiscall Gear_Level_ctor(void *this, int param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: Gear* — the gear obstacle being constructed
 *   param_1: int — parent Scene pointer (accessed at +0x878→App for MeshWorld at +0x174)
 *   param_2: undefined4 — stored at this+0x10D4 (position X)
 *   param_3: undefined4 — stored at this+0x10D8 (position Y)
 *   param_4: undefined4 — stored at this+0x10DC (position Z)
 *
 * Description:
 * Constructor for Gear obstacles (rotating gear cogs found in multiple levels).
 * Uses vtable PTR_Gear_DeletingDtor (0x4D52B8). Inherits from Level_ctor (not Stands_ctor),
 * making Gear a direct Level subclass.
 *
 * Steps:
 *   1. Calls Level_ctor(this, App→+0x174) — base class init with MeshWorld
 *   2. Sets vtable to PTR_Gear_DeletingDtor (0x4D52B8)
 *   3. Stores position at +0x10D4/+0x10D8/+0x10DC, parent Scene at +0x10D0
 *   4. Sets +0x480→+0x10D0 = 1 (collision enabled flag on the level's sub-collision)
 *   5. Sets +0x10E0 = RNG_Rand(10) (random initial rotation angle 0-9)
 *   6. Sets +0x10F4 = 1 (active flag = true)
 *   7. Sets +0x10F8 = 400 (timer/rotation duration, ~6.67 seconds)
 *   8. Sets +0x10FC = RNG_Rand(50) + 100 (rotation speed, 100-150 range)
 *   9. Sets +0x10E4 = RNG_Rand(360) (random initial Y rotation, full circle)
 *   10. Calls ScoreDisplay_SetTime(this, 0) — initializes score display
 *
 * The random initial positions and speeds ensure each gear spins differently.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0: initial rotation angle (random 0-9)
 *   +0x10E4: Y rotation (random 0-359)
 *   +0x10F4: active flag (1)
 *   +0x10F8: timer/rotation duration (400)
 *   +0x10FC: rotation speed (random 100-150)
 *
 * Cross-references:
 *   Called from CreateExpertLevelObjects (0x40E5E2) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
