/*
 * Function: AthenaList_Append_v2
 * Address: 0x00453810
 * Signature: void __thiscall ...(void *this, int param_1)
 * Parameters:
 *   this: AthenaList* | param_1: int (element to append)
 *
 * Description:
 * Duplicate of AthenaList_Append_v1 at slightly different address. Same logic: sorted insert or realloc+append. 4+ calls.
 *
 * Struct offsets:
 *   +0x04 (count), +0x40C (heap array), +0x414 (sorted flag)
 *
 * Cross-references:
 *   4+ calls from RegKeyList_AppendStr, MusicChannel_LoadAndAppend, Ball_CreateTrailParticles
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
