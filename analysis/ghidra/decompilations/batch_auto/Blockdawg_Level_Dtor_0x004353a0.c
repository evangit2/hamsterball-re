/*
 * Function: Blockdawg_Level_Dtor
 * Address: 0x004353A0
 * Signature: void __fastcall Blockdawg_Level_Dtor(undefined4 *param_1)
 * Parameters:
 *   param_1: Blockdawg* this — the Blockdawg obstacle object to destroy.
 *
 * Description:
 * Destructor for Blockdawg obstacle objects. Steps:
 *   1. Sets vtable to PTR_Blockdawg_DeletingDtor (0x4D5458)
 *   2. Frees a heap allocation at +0x43F (via _free — likely a string or buffer)
 *   3. Frees Vec3List at +0x6A99 (collision face list — very high offset, large object)
 *   4. Calls _eh_vector_destructor_iterator_ twice to destroy 50-element (0x32) arrays:
 *      - Array at +0x376D, each element 0x418 bytes, using Vec3List_Free
 *      - Array at +0x441, each element 0x418 bytes, using Vec3List_Free
 *   5. Calls Level_Cleanup (frees CollisionLevel + other resources)
 *
 * The Blockdawg is a large object — two 50-element arrays of 0x418-byte structs
 * (total ~65KB) suggest it manages a grid or collection of collision sub-objects.
 *
 * Name verification: Vtable PTR_Blockdawg_DeletingDtor confirms name is correct.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D5458)
 *   +0x43F: heap buffer (freed via _free)
 *   +0x376D: array of 50 elements × 0x418 bytes (freed)
 *   +0x441: array of 50 elements × 0x418 bytes (freed)
 *   +0x6A99: Vec3List (freed)
 *
 * Cross-references:
 *   Called from Blockdawg_DeletingDtor (0x43AED3) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
