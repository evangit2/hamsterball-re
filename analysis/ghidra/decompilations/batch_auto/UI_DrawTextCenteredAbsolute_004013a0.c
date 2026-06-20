/*
 * Function: UI_DrawTextCenteredAbsolute
 * Address: 0x004013a0
 * Signature: void __thiscall UI_DrawTextCenteredAbsolute(void *this, ...)
 *
 * Description:
 * Draws centered text with shadow at an absolute screen position.
Measures the text width via Font_MeasureText, then subtracts half the width from
the X position to center it horizontally. Delegates to UI_DrawTextShadow for the
actual rendering. Same parameter structure as UI_DrawTextShadow.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __thiscall
UI_DrawTextCenteredAbsolute(void *this, byte *param_1, int param_2, int param_3,
                           int param_4, int param_5, undefined4 param_6, undefined4 param_7,
                           undefined4 param_8, undefined4 param_9, undefined4 param_10,
                           undefined4 param_11, undefined4 param_12, undefined4 param_13,
                           undefined4 param_14, undefined4 param_15)
{
  ulonglong uVar1;
  uVar1 = Font_MeasureText((char *)param_1);
  UI_DrawTextShadow(this, param_1, param_2 - (int)uVar1 / 2, param_3,
                    param_4, param_5, ppuVar2, uVar3, uVar4, uVar5, uVar6,
                    ppuVar7, uVar8, uVar9, uVar10, uVar11);
  return;
}
