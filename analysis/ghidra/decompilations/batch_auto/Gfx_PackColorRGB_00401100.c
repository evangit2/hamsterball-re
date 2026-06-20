/*
 * Function: Gfx_PackColorRGB
 * Address: 0x00401100
 * Signature: void __fastcall Gfx_PackColorRGB(void *param_1)
 *
 * Description:
 * Packs three float RGB components (passed via FPU stack, __ftol2 converts to int)
into a single 32-bit packed color value: (R << 16) | (G << 8) | B.
Calls Gfx_SetVTable8B to store the packed color. Alpha is forced to 0xFF via OR mask.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __fastcall Gfx_PackColorRGB(void *param_1)
{
  ulonglong uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  uVar1 = __ftol2();
  uVar2 = __ftol2();
  uVar3 = __ftol2();
  Gfx_SetVTable8B(param_1,(((uint)uVar1 | 0xffffff00) << 8 | (uint)uVar2 & 0xff) << 8 |
                          (uint)uVar3 & 0xff);
  return;
}
