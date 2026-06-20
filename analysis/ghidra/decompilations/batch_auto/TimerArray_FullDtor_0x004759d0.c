
void __fastcall TimerArray_FullDtor(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 local_5c [15];
  undefined4 uStack_20;
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004cdd30;
  local_c = ExceptionList;
  puVar2 = (undefined4 *)&stack0x00000008;
  puVar3 = local_5c;
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  local_4 = 1;
  ExceptionList = &local_c;
  Gfx_LoadMatrixFromStack((undefined4 *)(param_1 + 8));
  puVar2 = (undefined4 *)&stack0x0000004c;
  puVar3 = local_5c;
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  Gfx_LoadMatrixFromStack((undefined4 *)(param_1 + 0x4c));
  local_4 = local_4 & 0xffffff00;
  uStack_20 = 0x475a4e;
  Timer_Cleanup((undefined4 *)&stack0x00000004);
  local_4 = 0xffffffff;
  uStack_20 = 0x475a5f;
  Timer_Cleanup((undefined4 *)&stack0x00000048);
  ExceptionList = local_c;
  return;
}

