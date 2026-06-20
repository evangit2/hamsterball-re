/*
 * Function: Graphics_SetViewportZ
 * Address: 0x004539a0
 * Signature: void __thiscall ...(void *this, undefined4 param_1, undefined4 param_2)
 * Parameters:
 *   this: Graphics* | param_1: minZ | param_2: maxZ
 *
 * Description:
 * Sets viewport Z range. Stores minZ at +0x73C, maxZ at +0x740. Sets D3DRS_ZENABLE (0x30=0). If Z buffer flags (+0xA4 & 0x100 & 0x300000): sets ZFUNC (0x23=3), ZWRITEENABLE (0x24), ZMIN/ZMAX (0x24/0x25). 4+ calls.
 *
 * Struct offsets:
 *   +0x73C (minZ), +0x740 (maxZ), +0xA4 (flags), +0x7D7 (flag), +0x154 (D3D device)
 *
 * Cross-references:
 *   4+ calls from Graphics_RenderScene, Level_InitScene, Scene_SetupLevel9, OptionsMenu
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
