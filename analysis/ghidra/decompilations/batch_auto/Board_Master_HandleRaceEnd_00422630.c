/*
 * Function: Board_Master_HandleRaceEnd
 * Address: 0x00422630
 * Signature: Board_Master_HandleRaceEnd(...)
 *
 * Patterns: scene, board. Calls: Board_Master_HandleRaceEnd, Scene_HandleRaceEnd. Offsets: 4, Lines: 29
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Board_Master_HandleRaceEnd(int param_1)

{
  float fVar1;
  
  Scene_HandleRaceEnd(param_1);
  fVar1 = *(float *)(param_1 + 0x5840) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5840) = fVar1;
  fVar1 = *(float *)(param_1 + 0x5844) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5844) = fVar1;
  fVar1 = *(float *)(param_1 + 0x5848) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x5848) = fVar1;
  fVar1 = *(float *)(param_1 + 0x584c) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x584c) = fVar1;
  return;
}
