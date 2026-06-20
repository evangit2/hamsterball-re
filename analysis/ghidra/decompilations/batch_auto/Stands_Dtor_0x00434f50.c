/*
 * Function: Stands_Dtor
 * Address: 0x00434F50
 * Signature: void __fastcall Stands_Dtor(undefined4 *param_1)
 * Parameters:
 *   param_1: Stands* this — the stands object to destroy.
 *
 * Description:
 * Destructor for Stands objects (breakable arena stands).
 * 1. Sets vtable pointer to PTR_Stands_DeletingDtor (0x4D5390).
 * 2. Calls Level_Cleanup which handles freeing the CollisionLevel at +0x10D4
 *    and any other level-owned resources.
 *
 * Does NOT free the Stands memory itself — that's handled by the scalar
 * deleting destructor (Stands_DeletingDtor) which calls this then operator delete.
 *
 * Struct offsets:
 *   +0x00: vtable pointer (set to 0x4D5390)
 *
 * Cross-references:
 *   Called from Stands_DeletingDtor (0x43A6C3) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
