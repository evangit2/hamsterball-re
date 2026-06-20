/*
 * Function: RumbleBoard_UpArena_Dtor
 * Address: 0x00422c00
 * Signature: void __fastcall RumbleBoard_UpArena_Dtor(undefined4 *param_1)
 * Parameters: param_1: RumbleBoard* this — the Up Arena board to destroy
 *
 * Description:
 * Destructor for Up Arena board. Sets vtable, frees sub-mesh at +0x11F8 via vtable delete, then calls RumbleBoard_dtor for base cleanup.
 *
 * Struct offsets: +0x11F8 (sub-mesh)
 * Cross-references: 1 call, 0 data refs
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
