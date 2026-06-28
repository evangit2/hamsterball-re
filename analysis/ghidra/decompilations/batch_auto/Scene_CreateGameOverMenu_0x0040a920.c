/*
 * Function: Scene_CreateGameOverMenu
 * Address: 0x0040a920
 *
 * Description:
 *
Creates the appropriate game-over menu based on game state. Decision tree:
  1. If App+0x237 (arena mode): creates PauseArenaMenu_ctor
  2. Else checks App+0x220→+0x11 and +0x10 flags:
     - Both 0: creates PauseMenu_Ctor (standard pause)
     - +0x10 set: creates QuitRaceMenu
     - +0x11 set: creates QuitRace (quit to menu)
Allocates 0xCDC bytes for each menu. Adds to scene via Scene_AddObject.

Cross-refs: 3 calls — Scene_Update, OptionsMenu, Level_UpdateAndRender.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


/* ~1675 chars — see GhidraMCP for full body */

