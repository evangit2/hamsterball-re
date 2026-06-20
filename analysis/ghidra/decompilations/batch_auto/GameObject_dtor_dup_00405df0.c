/*
 * Function: GameObject_dtor (duplicate at 0x405DF0)
 * Address: 0x00405DF0
 * Signature: void __fastcall GameObject_dtor(undefined4 *param_1)
 *
 * Description:
 * This is a byte-for-byte duplicate of GameObject_dtor at 0x401480.
 * It performs the same 11-step base class cleanup: destroys child objects
 * via vtable dispatch, frees CollisionMesh, Vec3Lists, heap buffers,
 * resets matrices to identity, cleans up timers.
 *
 * The duplication exists because the compiler generated two copies of the
 * destructor — one at 0x401480 (referenced from various unwind handlers and
 * Ball_dtor2) and one at 0x405DF0 (referenced from GameObject_sub_dtor at
 * 0x405DD0). This is a common MSVC optimization where the compiler inlines
 * the destructor at multiple call sites to avoid indirect calls through
 * exception handling tables.
 *
 * Cross-references:
 *   - Called from GameObject_sub_dtor (0x405DD0) at 0x405DD3
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GameObject_dtor_00401480.c for the complete analysis — this is an exact duplicate */
