/*
 * Function: Scene_AddTextureToList
 * Address: 0x0044abf0
 * Signature: void __thiscall ...(void *this, char *param_1, int param_2)
 * Parameters:
 *   this: Scene* | param_1: char* texture name | param_2: int (object/value to append)
 *
 * Description:
 * Finds texture by name in AthenaList, appends param_2 to the texture's sub-list at +0x28 via AthenaList_Append. Sets dirty flag +0xCBC=1.
 *
 * Struct offsets:
 *   +0x88C (AthenaList), +0x890 (count), +0xC98 (array), +0xCBC (dirty). Item: +4 (name), +0x28 (sub-list)
 *
 * Cross-references:
 *   4+ calls from TimeTrialMenu_ctor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
