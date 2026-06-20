/*
 * Function: Level_SelectCameraProfile
 * Address: 0x0040aca0
 *
 * Description:
 *
Selects the camera profile based on arena unlock status. Same switch
structure as CheckArenaUnlock — maps level type to arena unlock flag.
If flag is 0 (locked): uses the "locked" camera profile (App+0x2AC→+4)
If flag is 1 (unlocked): uses the "unlocked" camera profile (App+0x2B0→+4)
The camera profile is stored at App+0x2A8→+4.

Cross-refs: 1 call — Level_InitScene.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


/* ~1271 chars — see GhidraMCP for full body */

