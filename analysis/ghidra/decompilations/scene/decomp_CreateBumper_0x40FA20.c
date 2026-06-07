// CreateBumper (0x40FA20) — Raw Ghidra decompilation
// Factory for BUMPER1-BUMPER4 objects (8 iterations)
// self-loads "levels\\level8"

void __fastcall CreateBumper(int *param_1)
{
  void *pvVar1;
  int iVar2;
  byte *pbVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9736;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar1 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\level8");
  }
  local_4 = 0xffffffff;
  param_1[0x22b] = (int)pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4 = 1;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,param_1[0x22b]);
  }
  local_4 = 0xffffffff;
  param_1[0x22c] = (int)pvVar1;
  Level_InitScene((int)param_1);
  iVar2 = 0;
  piVar4 = param_1 + 0x1913;
  piVar5 = param_1 + 0x10e3;
  do {
    iVar2 = iVar2 + 1;
    piVar6 = piVar5;
    pbVar3 = (byte *)AthenaString_Format(0x4f7448,(byte *)"N:BUMPER%d");
    Scene_CollectByNameFilter((void *)param_1[0x22b],pbVar3,piVar6);
    *piVar4 = 0;
    piVar4 = piVar4 + 1;
    piVar5 = piVar5 + 0x106;
  } while (iVar2 < 8);
  (**(code **)(*param_1 + 0x80))();
  ExceptionList = pvStack_c;
  return;
}