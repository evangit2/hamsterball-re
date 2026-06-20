/*
 * Function: GlassStands_Ctor
 * Address: 0x00438290
 * Signature: void * __thiscall GlassStands_Ctor(void *this, int param_1)
 * Parameters:
 *   this: GlassStands* — the glass stands object being constructed
 *   param_1: int — parent Scene pointer. Accesses param_1+0x878→App for multiple
 *     data pointers: +0x594 (MeshWorld for Stands_ctor), +0x59C (TipperVisual mesh 1),
 *     +0x598 (Stands mesh 2), +0x5A0 (TipperVisual mesh 3)
 *
 * Description:
 * Constructor for GlassStands objects — the breakable glass platforms in Glass Race (Level 12).
 * GlassStands is a composite object containing 3 sub-objects:
 *
 * Steps:
 *   1. Calls Stands_ctor(this, App→+0x594) — base class init with glass mesh
 *   2. Sets vtable to PTR_Glass_Level_scalar_dtor (0x4D4FF8)
 *   3. Stores parent Scene at +0x10D0
 *   4. Allocates TipperVisual (0x10D0 bytes) using App→+0x59C mesh, stores at +0x10D4
 *   5. Calls TipperVisual_Attach(+0x10D4, this) — attaches visual to this glass stand
 *   6. Allocates Stands (0x10D0 bytes) using App→+0x598 mesh, stores at +0x10D8
 *   7. Allocates another TipperVisual (0x10D0 bytes) using App→+0x5A0 mesh, stores at +0x10DC
 *   8. Calls TipperVisual_Attach(+0x10DC, +0x10D8) — attaches visual to sub-Stands
 *   9. Sets +0x10EC = 0 (timer/state)
 *   10. Sets +0x10F4 = 0 (triggered flag)
 *   11. Sets +0x10F0 = 0xBF800000 (float -1.0 — break direction)
 *
 * GlassStands uses the TipperVisual system — when glass breaks, the TipperVisual
 * plays a tipping/falling animation for the glass shards.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: TipperVisual pointer (glass top mesh, owned)
 *   +0x10D8: Stands pointer (glass base mesh, owned)
 *   +0x10DC: TipperVisual pointer (glass bottom mesh, owned)
 *   +0x10EC: timer/state (0)
 *   +0x10F0: break direction (-1.0)
 *   +0x10F4: triggered flag (0)
 *
 * Cross-references:
 *   Called from 0x40DABA — UNCONDITIONAL_CALL (Scene_SetupLevelCascade/Glass setup)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
