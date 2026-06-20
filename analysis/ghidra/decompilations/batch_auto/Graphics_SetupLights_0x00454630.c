/*
 * Function: Graphics_SetupLights
 * Address: 0x00454630
 * Signature: void __fastcall ...(void *param_1)
 * Parameters:
 *   param_1: Graphics* this
 *
 * Description:
 * Sets up D3D lighting with 8 light slots. Configures directional lights with type=1, diffuse colors, directions. Sets D3DRS_LIGHTING(0x1A=0x454657), D3DRS_AMBIENT(0x2A), D3DRS_SPECULARENABLE(0x1B). 2 calls.
 *
 * Struct offsets:
 *   +0x154 (D3D device→vtable+200: SetRenderState, SetLight)
 *
 * Cross-references:
 *   2 calls from Graphics_Initialize, Graphics_RenderScene
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
