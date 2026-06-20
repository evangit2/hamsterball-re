/*
 * Function: Spinner_Level_Dtor
 * Address: 0x00435330
 * Signature: void __fastcall Spinner_Level_Dtor(undefined4 *param_1)
 * Parameters:
 *   param_1: Spinner* this — the spinner obstacle object to destroy.
 *
 * Description:
 * Destructor for Spinner obstacle objects (rotating spinners in levels like
 * Neon Race). Steps:
 *   1. Sets vtable to PTR_Spinner_DeletingDtor (0x4D53F8) — confirms this is a Spinner
 *   2. If sub-object at +0x548 is non-null, calls its vtable[0x48] (likely a sub-mesh or animation cleanup)
 *   3. Frees Vec3List at +0x442 (collision face list)
 *   4. Calls Level_Cleanup (frees CollisionLevel at +0x10D4 and other resources)
 *
 * Name verification: Vtable PTR_Spinner_DeletingDtor confirms "Spinner" is correct.
 * Related: GameLevel_ctor at 0x4351F0 also uses this vtable — it constructs Spinner objects.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D53F8)
 *   +0x442: Vec3List (collision faces, freed)
 *   +0x548: sub-object pointer (cleaned via vtable[0x48] if non-null)
 *
 * Cross-references:
 *   Called from Spinner_DeletingDtor (0x43A6E3) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
