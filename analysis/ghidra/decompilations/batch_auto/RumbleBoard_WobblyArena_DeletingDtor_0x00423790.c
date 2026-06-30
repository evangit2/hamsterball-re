/*
 * Function: ArenaBoard_Wobbly_DeletingDtor
 * Address: 0x00423790
 * Signature: void __fastcall ArenaBoard_Wobbly_DeletingDtor(undefined4 *param_1)
 * Parameters: param_1: RumbleBoard* this — the Wobbly Arena board to destroy
 *
 * Description:
 * Scalar deleting destructor for Wobbly Arena board. Sets vtable, frees sub-mesh at +0x11F8 via vtable delete, then calls ArenaBoard_dtor for base cleanup.
 *
 * Struct offsets: +0x11F8 (sub-mesh)
 * Cross-references: 1 call, 0 data refs
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
