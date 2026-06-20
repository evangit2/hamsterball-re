/*
 * Function: UIListItem_ctor
 * Address: 0x004490a0
 * Signature: int __fastcall ...(int param_1)
 * Parameters:
 *   param_1: UIListItem* this — being constructed
 *
 * Description:
 * Constructor for UIListItem (0x444 bytes). Calls Vec3_Init on +0x08 (position), AthenaList_Init on +0x28 (sub-items). 4 call sites.
 *
 * Struct offsets:
 *   +0x08 (Vec3 position), +0x28 (AthenaList sub-items)
 *
 * Cross-references:
 *   4 calls from UIList_AddItem, UIList_AddSpacer, UIList_AddItemWithFormat
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
