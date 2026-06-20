/*
 * Function: TourneyMenu_Quit
 * Address: 0x004506a0
 * Signature: void __fastcall ...(int *param_1)
 * Parameters:
 *   param_1: TourneyMenu* this (as int*) — contains App at param_1[0x21E]
 *
 * Description:
 * Quits tournament menu. Cleans up linked scene via vtable+0x40. Sets Gfx cull mode. Dispatches render mode change. Sets exit state=3 (Gfx+0x708). Calls App_ShowResults(0), App_ShowMainMenu. 1 DATA ref.
 *
 * Struct offsets:
 *   param_1[0x21E] (App→+0x174: Gfx), Gfx+0x7D2 (cull), Gfx+0x708 (state=3), Gfx+0x154 (render dispatch)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D8444
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
