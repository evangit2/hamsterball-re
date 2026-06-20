/*
 * Function: RegisterDialog_HandleKey
 * Address: 0x00448890
 * Signature: void __thiscall ...(void *this, void *param_1)
 * Parameters:
 *   this: RegisterDialog* | param_1: void* (virtual key code)
 *
 * Description:
 * Handles keyboard input for serial key entry dialog. VK_BACK=erase char, VK_TAB=switch fields, VK_HOME/END=cursor pos, VK_LEFT/RIGHT=move cursor, VK_DELETE=erase, Ctrl+V=paste. Two text fields at +0x1098 and +0x10B4. Cursor at +0x1158.
 *
 * Struct offsets:
 *   +0x1108 (active field), +0x1098 (field 1), +0x10B4 (field 2), +0x1120/+0x113C (field data), +0x1158 (cursor pos)
 *
 * Cross-references:
 *   4 DATA refs from vtables
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
