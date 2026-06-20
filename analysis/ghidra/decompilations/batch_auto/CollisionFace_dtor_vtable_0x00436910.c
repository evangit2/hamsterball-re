/*
 * Function: CollisionFace_dtor_vtable
 * Address: 0x00436910
 * Signature: void __fastcall CollisionFace_dtor_vtable(undefined4 *param_1)
 * Parameters:
 *   param_1: CollisionFace* this — the collision face to destruct
 *
 * Description:
 * Destructor for CollisionFace objects. Simply sets the vtable pointer to
 * PTR_LAB_004D5768 (same as constructor's vtable assignment). This is a
 * trivial destructor — CollisionFace has no heap-allocated members that need
 * explicit freeing (its vertices and normals are inline floats).
 *
 * Called from BoardLevel_Up_Dtor when iterating the CollisionFace list and
 * freeing each face individually.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D5768)
 *
 * Cross-references:
 *   Called from BoardLevel_Up_Dtor (0x4205B5) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
