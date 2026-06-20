/*
 * Function: UIList_SetColorsByName
 * Address: 0x0044a970
 * Signature: void __thiscall ...(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, char *param_6)
 * Parameters:
 *   this: UIList* | param_2-5: color values (4 components) | param_6: char* item name to match
 *
 * Description:
 * Iterates AthenaList items, matches by __stricmp on item name (+4). Sets 4 color components at item+0xC/+0x10/+0x14/+0x18. 3 calls.
 *
 * Struct offsets:
 *   +0x88C (AthenaList), +0x890 (count), +0xC98 (array), +0x894 (iter index). Item: +4 (name), +0xC/+0x10/+0x14/+0x18 (colors)
 *
 * Cross-references:
 *   3 calls from OptionsMenu_RenderControls, OptionsMenu_ctor, Tourney_SetCurrentLevel
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
