/*
 * Function: Matrix_Identity
 * Address: 0x00453200
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Matrix* this — sets vtable only
 *
 * Description:
 * Sets matrix vtable to 0x4CF300 (identity/default vtable). Minimal init. 4+ calls from dtors and cleanup functions.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4CF300)
 *
 * Cross-references:
 *   4+ calls from GameObject_dtor, Scene_dtor, UIList_Cleanup, SceneObject_BaseClear
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
