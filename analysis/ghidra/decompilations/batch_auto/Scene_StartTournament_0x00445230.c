/*
 * Function: Scene_StartTournament
 * Address: 0x00445230
 * Signature: void __fastcall Scene_StartTournament(int *param_1)
 * Parameters:
 *   param_1: Scene* this (as int*) — the scene starting a tournament
 *
 * Description:
 * Initiates tournament mode. Steps:
 *   1. Calls vtable+0x40 on current scene and linked scene (param_1[0x22E])
 *   2. Sets MusicPlayer tempo: +0x534 → 1.0 (normal), +0x53C → 0.5 (half speed)
 *   3. Allocates TourneyMenu (0x111C bytes) via operator_new
 *   4. Calls TourneyMenu_ctor with App (param_1[0x21E]) and mode flag 1
 *   5. Adds TourneyMenu to Scene via Scene_AddObject
 *   6. Calls Tourney_AdvanceRound to start first round
 *   7. Calls Tourney_SetCurrentLevel with level from App→+0x220→+0x08
 *   8. Sets App→+0x5E4 = 0 (reset tournament score)
 *
 * Struct offsets:
 *   param_1[0x21E]: App pointer
 *   App+0x184: Scene_AddObject list
 *   App+0x220: level data → +0x08: current level index
 *   App+0x534: MusicPlayer #1 (tempo 1.0)
 *   App+0x53C: MusicPlayer #2 (tempo 0.5)
 *   App+0x5E4: tournament score (reset to 0)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D63B0 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
