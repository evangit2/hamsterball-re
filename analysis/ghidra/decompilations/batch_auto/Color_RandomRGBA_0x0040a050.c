/*
 * Function: Color_RandomRGBA
 * Address: 0x0040a050
 *
 * Description:
 *
Generates a random RGBA color. Takes 4 random float values from the FPU stack
(presumably pre-loaded by RNG_Rand), converts each to an 8-bit integer via __ftol2,
and packs them into a 32-bit color: (A<<24)|(R<<16)|(G<<8)|B.

Cross-refs: 7 calls — Sprite_DrawRect, Gfx_DrawQuadRandomColor, Graphics_DrawScreenRect.
Used for colorful visual effects like random-colored quads.

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

