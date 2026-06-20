/*
 * Function: UIList_AddItem
 * Address: 0x004492d0
 * Signature: void __thiscall ...(void *this, undefined4 param_1, char *param_2, int param_3, int param_4)
 * Parameters:
 *   this: UIList* | param_1: undefined4 (callback data) | param_2: char* item name | param_3: int (height) | param_4: int (spacing)
 *
 * Description:
 * Adds a simple UIList item. Allocates UIListItem (0x444 bytes), copies name string, creates AthenaString. Appends to AthenaLists. Updates scroll metrics at +0xCB0/+0xCB4, sets dirty flag +0xCBC=1. 1+ call from MainMenu_ctor.
 *
 * Struct offsets:
 *   +0x44C (AthenaList), +0x88C (items), +0xCB0 (min scroll), +0xCB4 (total height), +0xCAC (max), +0xCBC (dirty=1)
 *
 * Cross-references:
 *   1+ call from MainMenu_ctor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
