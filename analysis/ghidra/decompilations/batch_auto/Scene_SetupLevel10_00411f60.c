/*
 * Function: Scene_SetupLevel10
 * Address: 0x00411f60
 * Signature: void __fastcall Scene_SetupLevel10(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_SetupLevel10, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaString_Format, Scene_CollectByNameFilter, AthenaList_NextIndex. Offsets: 19, Lines: 101
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_SetupLevel10(int *param_1)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  byte *pbVar4;
  int iVar5;
  int *piVar6;
  int *piVar7;
  undefined4 *puVar8;
  int *piVar9;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9736;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar2 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\level10");
  }
  local_4 = 0xffffffff;
  param_1[0x22b] = (int)pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = 1;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = CollisionLevel_ctorWithLevel(pvVar2,param_1[0x22b]);
  }
  local_4 = 0xffffffff;
  param_1[0x22c] = (int)pvVar2;
  Level_InitScene((int)param_1);
  iVar3 = 0;
  piVar7 = param_1 + 0x14ff;
  piVar6 = param_1 + 0x10e7;
  do {
    iVar3 = iVar3 + 1;
    piVar9 = piVar6;
    pbVar4 = (byte *)AthenaString_Format(0x4f7448,(byte *)"N:BUMPER%d");
    Scene_CollectByNameFilter((void *)param_1[0x22b],pbVar4,piVar9);
    *piVar7 = 0;
    piVar7 = piVar7 + 1;
    piVar6 = piVar6 + 0x106;
  } while (iVar3 < 4);
  (**(code **)(*param_1 + 0x80))();
  if (*(int *)(param_1[0x21e] + 0x23c) == 0) {
    iVar3 = AthenaList_NextIndex((int)(param_1 + 0x150a));
    param_1[iVar3 + 0x150c] = 0;
    if (param_1[0x150b] < 1) {
      iVar5 = 0;
    }
    else {
      iVar5 = *(int *)param_1[0x160d];
      param_1[iVar3 + 0x150c] = 1;
    }
    while (iVar5 != 0) {
      Rotator_RemoveAndFree(iVar5);
      iVar1 = param_1[iVar3 + 0x150c];
      if (param_1[0x150b] <= iVar1) break;
      iVar5 = *(int *)(param_1[0x160d] + iVar1 * 4);
      param_1[iVar3 + 0x150c] = iVar1 + 1;
    }
  }
  iVar5 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar3 = *(int *)(param_1[0x22b] + 0x480);
  *(undefined4 *)(iVar3 + 0x89c + iVar5 * 4) = 0;
  if (*(int *)(iVar3 + 0x898) < 1) {
    puVar8 = (undefined4 *)0x0;
  }
  else {
    puVar8 = (undefined4 *)**(undefined4 **)(iVar3 + 0xca0);
    *(undefined4 *)(iVar3 + 0x89c + iVar5 * 4) = 1;
  }
  while (puVar8 != (undefined4 *)0x0) {
    iVar3 = __strnicmp((char *)*puVar8,"TarBubble",9);
    if (iVar3 == 0) {
      AthenaList_Append(param_1 + 0x1719,(int)puVar8);
    }
    iVar3 = *(int *)(param_1[0x22b] + 0x480);
    iVar1 = *(int *)(iVar3 + 0x89c + iVar5 * 4);
    if (*(int *)(iVar3 + 0x898) <= iVar1) break;
    puVar8 = *(undefined4 **)(*(int *)(iVar3 + 0xca0) + iVar1 * 4);
    *(int *)(iVar3 + 0x89c + iVar5 * 4) = iVar1 + 1;
  }
  if (*(char *)(param_1[0x21e] + 0x234) != '\0') {
    AthenaList_Append(param_1 + 0x335,param_1[0x10dd]);
    AthenaList_Append((void *)(*(int *)(param_1[0x22b] + 0x480) + 0x1c),param_1[0x10dd]);
    AthenaList_Append(param_1 + 0x43b,param_1[0x10de]);
    AthenaList_Append((void *)(param_1[0x22c] + 0x18),param_1[0x10de]);
  }
  ExceptionList = pvStack_c;
  return;
}
