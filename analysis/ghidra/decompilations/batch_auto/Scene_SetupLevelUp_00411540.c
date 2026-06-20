/*
 * Function: Scene_SetupLevelUp
 * Address: 0x00411540
 * Signature: void __fastcall Scene_SetupLevelUp(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_SetupLevelUp, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaList_NextIndex, __strnicmp, AthenaString_SprintfToBuffer. Offsets: 9, Lines: 113
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_SetupLevelUp(int *param_1)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
  float fVar5;
  int *piVar6;
  undefined1 *puVar7;
  char *pcVar8;
  float fVar9;
  undefined1 *puVar10;
  char *pcVar11;
  float fVar12;
  char *pcVar13;
  char *pcVar14;
  int aiStack_218 [3];
  char acStack_20c [256];
  char acStack_10c [256];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c996a;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar2 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\levelup");
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
  (**(code **)(*param_1 + 0x80))();
  if (param_1 + 0x10d3 != aiStack_218) {
    param_1[0x10d3] = 0;
    param_1[0x10d4] = 0x42480000;
    param_1[0x10d5] = 0;
  }
  param_1[0xa70] = 0x44610000;
  iVar3 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar4 = *(int *)(param_1[0x22b] + 0x480);
  *(undefined4 *)(iVar4 + 0x89c + iVar3 * 4) = 0;
  if (*(int *)(iVar4 + 0x898) < 1) {
    piVar6 = (int *)0x0;
  }
  else {
    piVar6 = (int *)**(undefined4 **)(iVar4 + 0xca0);
    *(undefined4 *)(iVar4 + 0x89c + iVar3 * 4) = 1;
  }
  while( true ) {
    if (piVar6 == (int *)0x0) {
      ExceptionList = pvStack_c;
      return;
    }
    iVar4 = __strnicmp((char *)*piVar6,"VAC-IN",6);
    if (iVar4 == 0) {
      fVar5 = (float)(*piVar6 + 6);
      pcVar13 = acStack_10c;
      pcVar14 = "VAC-OUT%s";
      AthenaString_SprintfToBuffer(pcVar13,(byte *)"VAC-OUT%s");
      puVar7 = &stack0xfffffdb8;
      AthenaString_SprintfToBuffer(acStack_20c,(byte *)"VAC-VEC%s");
      pvVar2 = operator_new(0x38);
      local_4 = 2;
      if (pvVar2 == (void *)0x0) {
        pvVar2 = (void *)0x0;
      }
      else {
        puVar10 = &stack0xfffffdc4;
        fVar12 = 0.0;
        pcVar11 = acStack_20c;
        AthenaHashTable_Lookup
                  ((void *)param_1[0x22b],(undefined4 *)&stack0xfffffdc4,pcVar11,(undefined1 *)0x0);
        fVar9 = 0.0;
        pcVar8 = acStack_10c;
        AthenaHashTable_Lookup
                  ((void *)param_1[0x22b],(undefined4 *)&stack0xfffffdb8,pcVar8,(undefined1 *)0x0);
        if ((int *)&stack0xfffffdac != piVar6 + 1) {
          puVar7 = (undefined1 *)piVar6[1];
          pcVar8 = (char *)piVar6[2];
          fVar9 = (float)piVar6[3];
        }
        pvVar2 = CollisionFace_ctor(pvVar2,param_1,(float)puVar7,(float)pcVar8,fVar9,(float)puVar10,
                                    (float)pcVar11,fVar12,(float)pcVar13,(float)pcVar14,fVar5);
      }
      local_4 = 0xffffffff;
      AthenaList_Append(param_1 + 0x10db,(int)pvVar2);
    }
    iVar4 = *(int *)(param_1[0x22b] + 0x480);
    iVar1 = *(int *)(iVar4 + 0x89c + iVar3 * 4);
    if (*(int *)(iVar4 + 0x898) <= iVar1) break;
    piVar6 = *(int **)(*(int *)(iVar4 + 0xca0) + iVar1 * 4);
    *(int *)(iVar4 + 0x89c + iVar3 * 4) = iVar1 + 1;
  }
  ExceptionList = pvStack_c;
  return;
}
