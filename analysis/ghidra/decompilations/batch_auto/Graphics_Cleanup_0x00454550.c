/*
 * Function: Graphics_Cleanup
 * Address: 0x00454550
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Graphics* this
 *
 * Description:
 * Full cleanup for Graphics. Frees AthenaList (+0x2E4), texture path (+0x7D8), D3D device (+0x154→vtable+8), D3DX handler (+0x7C→vtable+8), D3DX_ErrorHandler (+0x748). Saves quality settings. Matrix_Identity, Timer_Cleanup, Vec3List_Free. 1 call from Graphics_dtor.
 *
 * Struct offsets:
 *   +0x154 (D3D device), +0x748 (D3DX_ErrorHandler), +0x74C (Timer), +0x7D8 (texture path), +0x2E4 (AthenaList)
 *
 * Cross-references:
 *   1 call from Graphics_dtor (0x455363)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
