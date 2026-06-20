/*
 * Function: App_CreateConfirmMenu
 * Address: 0x004469E0
 * Signature: void __fastcall App_CreateConfirmMenu(int param_1)
 * Parameters:
 *   param_1: Scene/App object — contains App pointer at +0x878
 *
 * Description:
 * Allocates and creates a ConfirmMenu (0xCFC bytes) via operator_new.
 * Calls ConfirmMenu_ctor with App (param_1+0x878) and mode flag 1.
 * Adds menu to scene via Scene_AddObject(App→+0x184).
 *
 * Struct offsets:
 *   +0x878: App pointer (→+0x184: Scene object list)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D69B8 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
