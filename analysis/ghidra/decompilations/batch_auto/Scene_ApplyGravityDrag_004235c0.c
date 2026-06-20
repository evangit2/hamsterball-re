/*
 * Function: Scene_ApplyGravityDrag
 * Address: 0x004235c0
 * Signature: Scene_ApplyGravityDrag(...)
 *
 * Patterns: scene. Calls: Scene_ApplyGravityDrag, Scene_HandleRaceEnd. Offsets: 5, Lines: 34
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_ApplyGravityDrag(int param_1)

{
  float fVar1;
  
  Scene_HandleRaceEnd(param_1);
  fVar1 = *(float *)(param_1 + 0x5c58) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5c58) = fVar1;
  fVar1 = *(float *)(param_1 + 0x5c5c) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5c5c) = fVar1;
  fVar1 = *(float *)(param_1 + 0x5c60) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5c60) = fVar1;
  fVar1 = *(float *)(param_1 + 0x5c64) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5c64) = fVar1;
  fVar1 = *(float *)(param_1 + 0x5c68) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5c68) = fVar1;
  return;
}
