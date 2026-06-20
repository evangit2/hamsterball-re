/*
 * Function: Vec3List_Free
 * Address: 0x00453250
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Vec3List* this
 *
 * Description:
 * Frees Vec3List. Sets vtable to 0x4D875C. If +0x40C (0x103 DWORDs) is non-NULL, calls _free. Sets +0x40C=0. 4+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4D875C), +0x40C (heap pointer→freed)
 *
 * Cross-references:
 *   4+ calls from GameObject_dtor, MeshBuffer_dtor, SceneObject_dtor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
