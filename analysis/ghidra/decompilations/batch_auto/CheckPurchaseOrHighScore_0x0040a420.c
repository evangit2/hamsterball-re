/*
 * Function: CheckPurchaseOrHighScore
 * Address: 0x0040a420
 *
 * Description:
 *
Checks if the game is purchased or shows high score/purchase menu.
Logic:
  1. Checks purchase state via App vtable
  2. If not purchased OR demo mode: creates ConfirmMenu (purchase prompt)
  3. If purchased: checks Game_SetInProgress
     - If game in progress: creates HighScoreEntry (score saving)
     - If not: creates ScoreDisplay with "BUY HAMSTERBALL" message
All menus added to Scene via Scene_AddObject.

Cross-refs: 1 call — TourneyMenu_TickWithRank.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


/* ~1867 chars — see GhidraMCP for full body */

