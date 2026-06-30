/*
 * Function: Stands_CtorWithCollision
 * Address: 0x00434E60
 * Signature: void * __thiscall Stands_CtorWithCollision(void *this, int param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: Stands* — the stands object being constructed
 *   param_1: int — pointer to parent Scene (Board). Accessed at param_1+0x878 to reach App, then App+0x5C8 for a data pointer passed to Stands_ctor.
 *   param_2: undefined4 — stored at this+0x10D8 (likely position X or rotation data)
 *   param_3: undefined4 — stored at this+0x10DC (likely position Y or rotation data)
 *   param_4: undefined4 — stored at this+0x10E0 (likely position Z or rotation data)
 *
 * Description:
 * Constructor for Stands objects (breakable glass/wooden stands in arenas).
 * 1. Calls Stands_ctor with a pointer from App→Scene+0x878→+0x5C8 (likely MeshWorld or GfxSystem pointer).
 * 2. Sets vtable to PTR_Stands_DeletingDtor (0x4D5390).
 * 3. Stores position/rotation params at +0x10D8/+0x10DC/+0x10E0.
 * 4. Stores parent Scene pointer at +0x10D0.
 * 5. Allocates a CollisionLevel (0x10D0 bytes) via operator_new and CollisionLevel_ctorWithLevel
 *    — this creates the stands' own collision mesh from the level data.
 * 6. Copies a field from this+0x434 to CollisionLevel+0x434 (likely arena index or difficulty).
 * 7. Clears CollisionLevel+0x431 (collision enabled flag = 0).
 * 8. Initializes: +0x10E8=0 (break state), +0x10F0=0xFFFFFFFF (no last breaker), +0x10E4=3 (stand type/health=3).
 *
 * Struct offsets:
 *   this+0x10D0: parent Scene pointer
 *   this+0x10D4: CollisionLevel pointer (owned)
 *   this+0x10D8/+0x10DC/+0x10E0: position/rotation data
 *   this+0x10E4: stand health/type (3)
 *   this+0x10E8: break state (0 = intact)
 *   this+0x10F0: last breaker ball index (0xFFFFFFFF = none)
 *
 * Cross-references:
 *   Called from 0x40ECBA — UNCONDITIONAL_CALL (from ExpertCollisionEvents factory dispatch)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
