/*
 * Function: Scene_SetupLevelCascade
 * Address: 0x004110d0
 * Signature: void __fastcall Scene_SetupLevelCascade(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_SetupLevelCascade, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaString_Format, Scene_CollectByNameFilter. Offsets: 5, Lines: 53
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_SetupLevelCascade(int *param_1)

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
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\levelcascade");
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
  (**(code **)(*param_1 + 0x80))();
  iVar2 = 0;
  piVar4 = param_1 + 0x190b;
  piVar5 = param_1 + 0x10db;
  do {
    iVar2 = iVar2 + 1;
    piVar6 = piVar5;
    pbVar3 = (byte *)AthenaString_Format(0x4f7448,(byte *)"N:BUMPER%d");
    Scene_CollectByNameFilter((void *)param_1[0x22b],pbVar3,piVar6);
    *piVar4 = 0;
    piVar4 = piVar4 + 1;
    piVar5 = piVar5 + 0x106;
  } while (iVar2 < 8);
  ExceptionList = pvStack_c;
  return;
}
