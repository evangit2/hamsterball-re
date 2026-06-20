/*
 * Function: Glass_Level_ctor
 * Address: 0x004384A0
 * Signature: void * __thiscall Glass_Level_ctor(void *this, undefined4 param_1, int param_2)
 * Parameters:
 *   this: Glass_Level* — the Glass level object being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: int — passed to Stands_ctor (parent scene for MeshWorld access)
 *
 * Description:
 * Constructor for Glass level objects (Glass Race / Level 12). Uses vtable
 * PTR_Impossible_Level_scalar_dtor (0x4D5060) — shares vtable with Impossible
 * level, suggesting Glass and Impossible levels share the same base class.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_2) — base class init
 *   2. Sets vtable to PTR_Impossible_Level_scalar_dtor (0x4D5060)
 *   3. Stores parent Scene at +0x10D0
 *   4. Initializes position to (0, 0, 0) at +0x10D8/+0x10DC/+0x10E0
 *   5. Sets +0x10E4 = 0, +0x10E8 = -1.0 (rotation/state)
 *   6. Creates CollisionLevel (0x10D0 bytes) at +0x10D4
 *   7. Constructs a Timer array (2 elements × 0x44 bytes) using _eh_vector_constructor_iterator_
 *   8. Calls Gfx_ScaleY(-80.0) and Gfx_ScaleY(-100.0) — sets up glass wall height
 *   9. Allocates TimerArray (0x90 bytes) using App→+0x174, stores at +0x10EC
 *   10. Looks up "Chain1Bridge" in AthenaHashTable — finds chain bridge positions
 *   11. (Function continues beyond decompilation cutoff — sets up chain bridges)
 *
 * Name verification: Vtable PTR_Impossible_Level_scalar_dtor (0x4D5060) confirms
 * Glass_Level shares the Impossible_Level vtable. This is likely because both
 * levels use similar breakable/collapsible mechanics.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned)
 *   +0x10D8/+0x10DC/+0x10E0: position (0, 0, 0)
 *   +0x10E4: rotation/state (0)
 *   +0x10E8: break direction (-1.0)
 *   +0x10EC: TimerArray pointer (owned, 0x90 bytes)
 *
 * Cross-references:
 *   Called from 0x40D991 — UNCONDITIONAL_CALL (Glass level setup)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
