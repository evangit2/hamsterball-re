/*
 * Function: Blockdawg_ctor
 * Address: 0x0043C310
 * Signature: void * __thiscall Blockdawg_ctor(void *this, undefined4 param_1, int param_2, undefined4 param_3, undefined4 param_4, void *param_5, undefined4 param_6)
 * Parameters:
 *   this: Blockdawg* — being constructed (vtable = 0x4D5638, RumbleObject)
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene)
 *   param_2: int — stored at this+0x10D8 (position X)
 *   param_3: undefined4 — stored at this+0x10DC (position Y)
 *   param_4: undefined4 — stored at this+0x10E0 (position Z)
 *   param_5: void* — passed to Stands_ctor (parent for MeshWorld access)
 *   param_6: undefined4 — stored at this+0x10F0 (path data pointer)
 *
 * Description:
 * Constructor for Blockdawg obstacles (moving block enemies that follow paths).
 * Uses vtable PTR_ArenaObject_DeletingDtor (0x4D5638) — Blockdawg inherits from
 * RumbleObject, not directly from Stands.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_5) — base class init
 *   2. Sets vtable to PTR_ArenaObject_DeletingDtor (0x4D5638)
 *   3. Initializes Timer at +0x10F4
 *   4. Stores parent Scene at +0x10D0, position at +0x10D8/+0x10DC/+0x10E0
 *   5. Stores path data pointer at +0x10F0
 *   6. Creates CollisionLevel (0x10D0 bytes) at +0x10D4
 *   7. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   8. Initializes Timer, calls SceneObject_CallUpdate/Render
 *   9. Calculates path length from path data (data+4 and data+8 difference >> 2)
 *   10. Uses RNG_Rand with path_length - 2 for random starting position
 *   11. Sets +0x113C = 1.0 (speed multiplier)
 *   12. Sets +0x1138 = random initial value (continues beyond cutoff)
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned)
 *   +0x10D8/+0x10DC/+0x10E0: position (X/Y/Z)
 *   +0x10F0: path data pointer (defines movement path)
 *   +0x10F4: Timer (initialized)
 *   +0x1138: random initial value
 *   +0x113C: speed multiplier (1.0)
 *
 * Cross-references:
 *   Called from CreateLevelObjects (0x4125F7, 0x4126A6) — UNCONDITIONAL_CALL
 *   Called from 0x40FE58, 0x40FF03, 0x40FFB2 — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
