/*
 * Function: AthenaList_Append_v1
 * Address: 0x00453780
 * Signature: void __thiscall ...(void *this, int param_1)
 * Parameters:
 *   this: AthenaList* | param_1: int (element to append)
 *
 * Description:
 * Appends element to AthenaList. If sorted mode (+0x414!=0): calls AthenaList_SortedInsert. Else: first element _malloc(4), zero 256 entries. Subsequent: _realloc(array, count*4). Element stored at array[count-1]. 4+ calls.
 *
 * Struct offsets:
 *   +0x04 (count), +0x08 through +0x407 (256 entries), +0x40C (heap array), +0x414 (sorted mode flag)
 *
 * Cross-references:
 *   4+ calls from Graphics_FindOrCreateTexture, Ball_FallUpdate, MeshWorld
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
