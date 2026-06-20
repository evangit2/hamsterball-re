/*
 * Function: BoardLevel_Up_Dtor
 * Address: 0x00420550
 *
 * Description:
 *
Destructor for Up Race board. Sets vtable, iterates AthenaList at +0x10DB and
frees each CollisionFace object via CollisionFace_dtor_vtable + _free. Then Scene_dtor.
1 call. ~1506 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
