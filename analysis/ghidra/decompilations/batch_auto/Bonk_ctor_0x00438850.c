/*
 * Function: Bonk_ctor
 * Address: 0x00438850
 * Signature: void * __thiscall Bonk_ctor(void *this, int param_1, float param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: Bonk* — the Bonk obstacle being constructed (hammer/impact obstacle in Tower Race)
 *   param_1: int — parent Scene pointer. Accesses param_1+0x878→App for MeshWorld and other data
 *   param_2: float — position X, stored at this+0x10D4 and this+0x10E0 (with offset)
 *   param_3: undefined4 — position Y, stored at this+0x10D8 and this+0x10E4
 *   param_4: undefined4 — position Z, stored at this+0x10DC and this+0x10E8 (with offset)
 *
 * Description:
 * Constructor for Bonk obstacles — the hammer/impact obstacles in Tower Race (Level 5).
 * Despite Ghidra's label "Bonk_ctor", it loads the mesh "levels\\level5-bonk" via
 * MeshWorld_ctor, confirming it's a Tower Race (Level 5) obstacle. Uses vtable
 * PTR_WarmUp_Level_scalar_dtor (0x4D5120) — shares vtable with WarmUp level objects.
 *
 * Steps:
 *   1. Calls MeshWorld_ctor(this, App→+0x174, "levels\\level5-bonk") — loads the bonk mesh
 *   2. Sets vtable to PTR_WarmUp_Level_scalar_dtor (0x4D5120)
 *   3. Stores parent Scene at +0x10D0
 *   4. Stores position at +0x10D4/+0x10D8/+0x10DC (primary) and +0x10E0/+0x10E4/+0x10E8 (backup with offset)
 *   5. Adjusts backup position: X += _DAT_004CF9F8, Z -= _DAT_004CF9F8 (offset for collision area)
 *   6. Creates CollisionLevel (0x10D0 bytes) at +0x10F8
 *   7. Calls TipperVisual_Attach(CollisionLevel, this) — attaches visual
 *   8. Sets +0x10FC = 1 (active flag)
 *   9. Sets +0x1138 = 0 (state)
 *   10. Sets +0x10FD = 0 (triggered flag)
 *   11. Sets +0x1100 = 1000 (timer max — ~16.67 seconds at 60fps)
 *   12. Looks up "IMPACT" in AthenaHashTable → stores collision position at +0x10EC/+0x10F0/+0x10F4
 *   13. Looks up "HAMMERAREA1" in Scene→+0x8AC → stores hammer area bounds
 *   14. (Function continues — sets up additional hammer area lookups)
 *
 * Name verification: The "level5-bonk" mesh name confirms this is a Tower Race (L5)
 * obstacle. The vtable (PTR_WarmUp_Level_scalar_dtor) is shared, suggesting WarmUp
 * and Bonk share the same level base class.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0/+0x10E4/+0x10E8: offset position (X+offset, Y, Z-offset)
 *   +0x10EC/+0x10F0/+0x10F4: impact position (from IMPACT hash lookup)
 *   +0x10F8: CollisionLevel pointer (owned)
 *   +0x10FC: active flag (1)
 *   +0x10FD: triggered flag (0)
 *   +0x1100: timer max (1000)
 *   +0x1138: state (0)
 *
 * Cross-references:
 *   Called from CreateExpertLevelObjects (0x40E2DA) — UNCONDITIONAL_CALL
 *   Called from CreateLevelObjects (0x4123A2) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
