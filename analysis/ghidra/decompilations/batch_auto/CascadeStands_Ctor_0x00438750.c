/*
 * Function: CascadeStands_Ctor
 * Address: 0x00438750
 * Signature: void * __thiscall CascadeStands_Ctor(void *this, undefined4 param_1, int param_2)
 * Parameters:
 *   this: CascadeStands* — the cascade stands object being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: int — passed to Stands_ctor (parent scene for MeshWorld access)
 *
 * Description:
 * Constructor for CascadeStands objects — the cascading/falling platforms in
 * Beginner Race (Level 2, also called "LevelCascade"). Uses vtable
 * PTR_Cascade_Level_scalar_dtor (0x4D50C0).
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_2) — base class init
 *   2. Sets vtable to PTR_Cascade_Level_scalar_dtor (0x4D50C0)
 *   3. Stores parent Scene at +0x10D0
 *   4. Initializes position to (0, 0, 0) at +0x10D8/+0x10DC/+0x10E0
 *   5. Sets +0x10E4 = 0 (rotation/state)
 *   6. Creates CollisionLevel (0x10D0 bytes) at +0x10D4
 *   7. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   8. Sets +0x10E8 = 0 (timer/state)
 *   9. Sets +0x10F0 = 0 (triggered flag)
 *   10. Sets +0x10F4 = 0 (activation timer)
 *   11. Sets +0x10EC = 0xBF800000 (float -1.0 — break/fall direction)
 *   12. Sets +0x1108 = 1 (is_cascade flag = true, marks as cascade-type stands)
 *
 * Name verification: Vtable PTR_Cascade_Level_scalar_dtor confirms "Cascade" is correct.
 * CascadeStands are the collapsible platforms in Beginner Race that fall away
 * after the ball passes over them.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned)
 *   +0x10D8/+0x10DC/+0x10E0: position (0, 0, 0)
 *   +0x10E4: rotation/state (0)
 *   +0x10E8: timer (0)
 *   +0x10EC: break direction (-1.0)
 *   +0x10F0: triggered flag (0)
 *   +0x10F4: activation timer (0)
 *   +0x1108: is_cascade flag (1)
 *
 * Cross-references:
 *   Called from 0x40D8CF — UNCONDITIONAL_CALL (Beginner/Cascade level setup)
 *   Called from 0x4146F3 — UNCONDITIONAL_CALL (CreateWobbly1 or similar)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
