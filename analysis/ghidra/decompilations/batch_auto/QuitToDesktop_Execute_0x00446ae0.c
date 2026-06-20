/*
 * Function: QuitToDesktop_Execute
 * Address: 0x00446AE0
 * Signature: void __fastcall QuitToDesktop_Execute(int param_1)
 * Parameters:
 *   param_1: QuitToDesktopDialog* this — the confirmed quit dialog
 *
 * Description:
 * Executes the "Quit To Desktop" action after user confirms. Steps:
 *   1. Calls vtable+0x40 on App→+0x115C (cleanup linked scene)
 *   2. Gets Gfx from App→+0x174, sets +0x7D2 = 0
 *   3. Calls Gfx_SetCullMode
 *   4. Dispatches render mode change (vtable+200, arg 0x16, mode 2 or 3)
 *   5. Sets Gfx+0x708 = 3 (exit state)
 *   6. Calls App_StartRace, App_ShowResults(0), App_ShowMainMenu
 *
 * Struct offsets:
 *   +0x878: App pointer
 *   App+0x115C: linked scene pointer
 *   App+0x174: Graphics object (+0x7D2: cull flag, +0x708: state, +0x154: render dispatch)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D6A10 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
