/*
 * Function: Mesh_Clear
 * Address: 0x00456390
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Mesh* this
 *
 * Description:
 * Clears mesh data. Sets vtable to 0x4D8E10. Iterates and _free's all elements in AthenaList at +0x18 (offset 6 DWORDs). Then frees AthenaList at +0x848 (offset 0x212 DWORDs). 3 refs.
 *
 * Struct offsets:
 *   +0x18 (AthenaList, offset 6), +0x848 (AthenaList, offset 0x212), vtable=0x4D8E10
 *
 * Cross-references:
 *   3 refs from Unwind, Mesh_FindClosestCollision, Mesh_DeletingDtor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
