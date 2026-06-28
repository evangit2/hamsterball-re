// ArenaSceneObj_Tick @ 0x0042B660
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall ArenaSceneObj_Tick(int param_1)

{
  float fVar1;
  
  NoOp();
  fVar1 = *(float *)(param_1 + 0x880) + _DAT_004cf524;
  *(float *)(param_1 + 0x880) = fVar1;
  if ((float)_DAT_004cf438 < fVar1) {
    *(undefined4 *)(param_1 + 0x880) = 0x3f400000;
  }
  fVar1 = *(float *)(param_1 + 0x884) - _DAT_004cf370;
  *(float *)(param_1 + 0x884) = fVar1;
  if (fVar1 < _DAT_004cf368) {
    *(undefined4 *)(param_1 + 0x884) = 0;
  }
  ToggleTimer_Tick(param_1 + 0x888);
  ToggleTimer_Tick(param_1 + 0x89c);
  return;
}