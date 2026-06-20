// Scene_HandleRaceEnd_ClampZoom @ 0x0041F7E0
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_HandleRaceEnd_ClampZoom(int param_1)

{
  float fVar1;
  
  Scene_HandleRaceEnd(param_1);
  fVar1 = *(float *)(param_1 + 0x644c) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x644c) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6450) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6450) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6454) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6454) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6458) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6458) = fVar1;
  fVar1 = *(float *)(param_1 + 0x645c) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x645c) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6460) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6460) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6464) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6464) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6468) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6468) = fVar1;
  return;
}