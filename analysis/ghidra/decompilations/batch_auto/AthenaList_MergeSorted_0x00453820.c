/*
 * Function: AthenaList_MergeSorted
 * Address: 0x00453820
 * Signature: void __thiscall ...(void *this, int param_1)
 * Parameters:
 *   this: AthenaList* (destination) | param_1: int (source AthenaList*)
 *
 * Description:
 * Merges source AthenaList into destination. If destination not sorted (+0x414==0): _realloc to combined size, memcpy source elements. If sorted: uses circular buffer at +0x408 (0-255), inserts via sorted insert. 4+ calls.
 *
 * Struct offsets:
 *   +0x04 (count), +0x408 (circular buffer index 0-255), +0x40C (heap array), +0x414 (sorted flag)
 *
 * Cross-references:
 *   4+ calls from MeshWorld_Parse, Scene_ComputeArenaLighting
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
