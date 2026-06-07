// CreateMouseTrap (0x40BF50) — Raw Ghidra decompilation
// Spawns mouse traps from MESHWORLD objects named "MOUSETRAP"

void __fastcall CreateMouseTrap(int param_1)
{
  int iVar1;
  int iVar2;
  int iVar3;
  void *this;
  int *piVar4;
  undefined4 *puVar5;
  int local_18;
  int local_14;
  int local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c963b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar2 = AthenaList_NextIndex(*(int *)(*(int *)(param_1 + 0x8ac) + 0x480) + 0x894);
  iVar3 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
  *(undefined4 *)(iVar3 + 0x89c + iVar2 * 4) = 0;
  if (*(int *)(iVar3 + 0x898) < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = (undefined4 *)**(undefined4 **)(iVar3 + 0xca0);
    *(undefined4 *)(iVar3 + 0x89c + iVar2 * 4) = 1;
  }
  while( true ) {
    if (puVar5 == (undefined4 *)0x0) {
      ExceptionList = local_c;
      return;
    }
    piVar4 = (int *)0x0;
    iVar3 = __stricmp((char *)*puVar5,"MOUSETRAP");
    if (iVar3 == 0) {
      this = operator_new(0x10f8);
      local_4 = 0;
      if (this != (void *)0x0) {
        piVar4 = TipperVisual_Level_Ctor(this,param_1);
      }
      local_4 = 0xffffffff;
      AthenaList_Append((void *)(param_1 + 0xcd4),(int)piVar4);
      AthenaList_Append((void *)(param_1 + 0x1930),(int)piVar4);
      AthenaList_Append((void *)(*(int *)(*(int *)(param_1 + 0x8ac) + 0x480) + 0x1c),(int)piVar4);
      AthenaList_Append((void *)(*(int *)(param_1 + 0x8b0) + 0x18),piVar4[0x435]);
      local_14 = puVar5[2];
      local_10 = puVar5[3];
      if (piVar4 + 0x437 != &local_18) {
        piVar4[0x437] = puVar5[1];
        piVar4[0x438] = local_14;
        piVar4[0x439] = local_10;
      }
      piVar4[0x43d] = (int)(_DAT_004cf44c - (float)puVar5[5]);
      (**(code **)(*piVar4 + 0x2c))();
    }
    iVar3 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
    iVar1 = *(int *)(iVar3 + 0x89c + iVar2 * 4);
    if (*(int *)(iVar3 + 0x898) <= iVar1) break;
    puVar5 = *(undefined4 **)(*(int *)(iVar3 + 0xca0) + iVar1 * 4);
    *(int *)(iVar3 + 0x89c + iVar2 * 4) = iVar1 + 1;
  }
  ExceptionList = local_c;
  return;
}