/*
 * Function: Gfx_SetVTable22
 * Address: 0x00453b20
 * Signature: void __thiscall ...(void *this, undefined4 param_1)
 * Parameters:
 *   this: Graphics* | param_1: undefined4 (stored at +0x738)
 *
 * Description:
 * Sets D3D render state 0x22 (D3DRS_SHADEMODE) via device vtable+200. Caches value at +0x738. 1 call from Level_InitScene.
 *
 * Struct offsets:
 *   +0x738 (shade mode cache), +0x154 (D3D device→vtable+200, state 0x22)
 *
 * Cross-references:
 *   1 call from Level_InitScene (0x40B175)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
