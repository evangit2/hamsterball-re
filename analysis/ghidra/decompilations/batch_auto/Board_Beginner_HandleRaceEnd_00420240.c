/*
 * Function: Board_Beginner_HandleRaceEnd
 * Address: 0x00420240
 * Signature: Board_Beginner_HandleRaceEnd(...)
 *
 * Patterns: scene, board. Calls: Board_Beginner_HandleRaceEnd, Scene_HandleRaceEnd. Offsets: 8, Lines: 49
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Board_Beginner_HandleRaceEnd(int param_1)

{
  float fVar1;
  
  Scene_HandleRaceEnd(param_1);
  fVar1 = *(float *)(param_1 + 0x642c) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x642c) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6430) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6430) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6434) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6434) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6438) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6438) = fVar1;
  fVar1 = *(float *)(param_1 + 0x643c) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x643c) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6440) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6440) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6444) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6444) = fVar1;
  fVar1 = *(float *)(param_1 + 0x6448) - (float)_DAT_004cf428;
  if (fVar1 <= (float)_DAT_004cf6a8) {
    fVar1 = (float)_DAT_004cf6a8;
  }
  *(float *)(param_1 + 0x6448) = fVar1;
  return;
}
