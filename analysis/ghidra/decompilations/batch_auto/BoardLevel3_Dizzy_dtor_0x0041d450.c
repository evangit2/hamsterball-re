/*
 * Function: BoardLevel3_Dizzy_dtor
 * Address: 0x0041d450
 *
 * Description:
 *
Destructor for Dizzy Race board. Sets vtable, frees 3 sub-mesh objects at
+0x10DB/+0x10DC/+0x10DD (via vtable delete), frees Vec3Lists at +0x11E4 and
+0x10DE, then calls Scene_dtor. 1 call. ~991 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
