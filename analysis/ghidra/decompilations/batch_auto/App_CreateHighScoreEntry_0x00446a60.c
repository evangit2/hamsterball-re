/*
 * Function: App_CreateHighScoreEntry
 * Address: 0x00446A60
 * Signature: void __fastcall App_CreateHighScoreEntry(int param_1)
 * Parameters:
 *   param_1: Scene/App object — contains App pointer at +0x878
 *
 * Description:
 * Allocates and creates a HighScoreEntry dialog (0x904 bytes) via operator_new.
 * Calls HighScoreEntry_ctor with App (param_1+0x878) and high score data
 * at App+0x5CC. Adds dialog to scene via Scene_AddObject(App→+0x184).
 *
 * Struct offsets:
 *   +0x878: App pointer (→+0x184: Scene objects, →+0x5CC: high score data)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D69C0 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
