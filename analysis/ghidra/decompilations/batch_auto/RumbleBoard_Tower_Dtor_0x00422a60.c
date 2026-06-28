/*
 * Function: ArenaBoard_Tower_Dtor
 * Address: 0x00422a60
 * Signature: void __fastcall ArenaBoard_Tower_Dtor(undefined4 *param_1)
 * Parameters: param_1: RumbleBoard* this — the Tower Arena board object to destroy
 *
 * Description:
 * Destructor for Tower Arena board. Sets vtable to PTR_ArenaBoard_Odd_DeletingDtor, frees 2 sub-mesh objects at +0x11F8 and +0x12FF via vtable delete, then calls Scene_dtor for base cleanup.
 *
 * Struct offsets: +0x11F8 (sub-mesh 1), +0x12FF (sub-mesh 2)
 * Cross-references: 1 call, 0 data refs
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
