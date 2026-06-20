/*
 * Function: Gfx_CallVTable8C
 * Address: 0x00453960
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: Graphics* this
 *
 * Description:
 * Thin wrapper: calls D3D device vtable+0x8C (likely EndScene or present). 1 call.
 *
 * Struct offsets:
 *   +0x154 (D3D device→vtable+0x8C)
 *
 * Cross-references:
 *   1 call from 0x46C256
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
