/*
 * Function: LoadRaceData
 * Address: 0x0040a120
 *
 * Description:
 *
Loads race timing data from 'racedata.xml'. Parses XML using MWParser_ReadTag,
searching for a tag matching the level name (param_1). Extracts:
  - TIME: target completion time (stored at scene+0x2998)
  - Challenge time (stored at scene+0x2990, computed as 9-attribute)
  - WEASEL: weasel time (scene+0x2994)
  - GOLD/SILVER/BRONZE: medal threshold times (scene+0x29A0/+0x29A4/+0x29A8)
  - Float difficulty multiplier (scene+0x299C)

Cross-refs: 15 calls — every BoardLevel*_ctor calls this to load level data.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


/* ~3452 chars — see GhidraMCP for full body */

