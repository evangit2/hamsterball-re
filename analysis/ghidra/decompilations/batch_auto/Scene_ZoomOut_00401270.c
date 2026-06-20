/*
 * Function: Scene_ZoomOut
 * Address: 0x00401270
 * Signature: void __fastcall Scene_ZoomOut(void *param_1)
 *
 * Description:
 * Zooms the camera out by subtracting a constant delta (_DAT_004cf308) from both
the near plane (this+0x790) and far plane (this+0x794) values, then calls
Graphics_SetProjection. Mirror of Scene_ZoomIn.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __fastcall Scene_ZoomOut(void *param_1)
{
  float fVar1;
  float fVar2;
  fVar1 = *(float *)((int)param_1 + 0x790) - (float)_DAT_004cf308;
  *(float *)((int)param_1 + 0x790) = fVar1;
  fVar2 = *(float *)((int)param_1 + 0x794) - (float)_DAT_004cf308;
  *(float *)((int)param_1 + 0x794) = fVar2;
  Graphics_SetProjection(param_1, fVar1, fVar2);
  return;
}
