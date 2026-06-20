/*
 * Function: Gfx_ResetRenderState
 * Address: 0x00453940
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: Graphics* this
 *
 * Description:
 * Resets render state: zeros +0x7C8 and +0x7CC. Calls D3D device vtable+0x88 (likely SetRenderState or BeginScene). 1 call from App_ResetFrame.
 *
 * Struct offsets:
 *   +0x7C8 (reset=0), +0x7CC (reset=0), +0x154 (D3D device→vtable+0x88)
 *
 * Cross-references:
 *   1 call from App_ResetFrame (0x46C209)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
