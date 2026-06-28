/*
 * Function: TourneyMenu_ctor
 * Address: 0x0044fda0
 * Signature: void * __thiscall ...(void *this, int param_1, char param_2)
 * Parameters:
 *   this: TourneyMenu* (vtable=0x4D83F0) | param_1: App | param_2: char (mode flag)
 *
 * Description:
 * Constructor for TourneyMenu. Calls SimpleMenu_ctor, overrides vtable to 0x4D83F0. ToggleTimer_Init at +0xCDC. Name 'Tourney Menu'. Calls Scene_UpdateChildren, UIList_AddItem for 'PLAY!' button. Checks tournament state at App+0x220 (+0x90=loss, +0x95=lost flag, +0x96=won flag). 4+ calls.
 *
 * Struct offsets:
 *   +0x868 ('Tourney Menu'), +0x878 (App→+0x220), +0xCDC (timer), +0xCF4 (flag), +0xCF8/+0xD00/+0xD04 (zeros), +0xCFC (level index)
 *
 * Cross-references:
 *   4+ calls from Tournament_AdvanceRace, TourneyMenu_Advance, TourneyMenu_LoadSaveAndShow
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
