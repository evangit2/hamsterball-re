/*
 * Function: BoardLevel_Master_Dtor
 * Address: 0x00420c10
 *
 * Description:
 *
Destructor for Master Race board. Sets vtable, frees 5+ sub-mesh objects at
+0x10E5/+0x10E6/+0x1504/+0x1505 (via vtable delete + _free), then Scene_dtor.
1 call. ~2098 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
