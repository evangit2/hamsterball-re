/*
 * Function: Gfx_SaveQualitySettings
 * Address: 0x00453f90
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: Graphics* this
 *
 * Description:
 * Saves texture quality, color mode, and safe mode to registry. Opens reg key at parent+0x54, writes DWORD 'Texture Quality' (+0x184), Bool 'ColorMode' (+0x18C), Bool 'SafeMode' (+0x7D7). 1 call from Graphics_Cleanup.
 *
 * Struct offsets:
 *   +0x5C (parent→+0x54: RegKey), +0x184 (Texture Quality), +0x18C (ColorMode), +0x7D7 (SafeMode)
 *
 * Cross-references:
 *   1 call from Graphics_Cleanup (0x4545D1)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
