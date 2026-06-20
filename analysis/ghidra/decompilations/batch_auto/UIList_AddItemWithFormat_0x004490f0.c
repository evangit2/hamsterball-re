/*
 * Function: UIList_AddItemWithFormat
 * Address: 0x004490f0
 * Signature: void __thiscall ...(void *this, char *param_1, char *param_2, undefined4 param_3-7, int param_8)
 * Parameters:
 *   this: UIList* | param_1: char* display text | param_2: char* item name | param_3-7: format params | param_8: int format type
 *
 * Description:
 * Adds a formatted UIList item. Allocates UIListItem (0x444 bytes), copies display text and name strings. Creates AthenaString (0x1C bytes) for formatting. Appends to AthenaList at +0x44C and +0x88C. Sets +0x441=1 (formatted flag). 4 calls from MPMenu_ctor.
 *
 * Struct offsets:
 *   +0x44C (AthenaList strings), +0x88C (AthenaList items)
 *
 * Cross-references:
 *   4 calls from MPMenu_ctor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
