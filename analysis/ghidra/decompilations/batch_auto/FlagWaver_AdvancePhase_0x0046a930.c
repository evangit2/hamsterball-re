
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FlagWaver_AdvancePhase(int param_1)

{
  *(float *)(param_1 + 0x28) = *(float *)(param_1 + 0x28) + _DAT_004cf55c;
  return;
}

