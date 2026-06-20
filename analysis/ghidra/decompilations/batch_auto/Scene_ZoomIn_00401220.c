/*
 * Function: Scene_ZoomIn
 * Address: 0x00401220
 * Signature: void __fastcall Scene_ZoomIn(void *param_1)
 *
 * Description:
 * Zooms the camera in by adding a constant delta (_DAT_004cf308) to both
the near plane (this+0x790) and far plane (this+0x794) values, then calls
Graphics_SetProjection to apply the new projection. param_1 is the Graphics object.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __fastcall Scene_ZoomIn(void *param_1)
{
  float fVar1;
  float fVar2;
  fVar1 = *(float *)((int)param_1 + 0x790) + (float)_DAT_004cf308;
  *(float *)((int)param_1 + 0x790) = fVar1;
  fVar2 = *(float *)((int)param_1 + 0x794) + (float)_DAT_004cf308;
  *(float *)((int)param_1 + 0x794) = fVar2;
  Graphics_SetProjection(param_1, fVar1, fVar2);
  return;
}
