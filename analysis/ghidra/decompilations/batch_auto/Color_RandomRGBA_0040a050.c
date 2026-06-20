/*
 * Function: Color_RandomRGBA
 * Address: 0x0040a050
 * Signature: uint Color_RandomRGBA(void)
 *
 * Patterns: none identified. Calls: Color_RandomRGBA, __ftol2. Offsets: 0, Lines: 14
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

uint Color_RandomRGBA(void)

{
  ulonglong uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  ulonglong uVar4;
  
  uVar1 = __ftol2();
  uVar2 = __ftol2();
  uVar3 = __ftol2();
  uVar4 = __ftol2();
  return (uint)uVar4 & 0xff |
         (((uint)uVar1 & 0xff | (int)uVar2 << 8) << 8 | (uint)uVar3 & 0xff) << 8;
}
