/*
 * Function: Scene_SetupLevel9
 * Address: 0x00410830
 * Signature: Scene_SetupLevel9(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_SetupLevel9, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaList_NextIndex, __strnicmp, AthenaList_Append. Offsets: 11, Lines: 105
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_SetupLevel9(int *param_1)

{
  float fVar1;
  int iVar2;
  void *pvVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  undefined4 *puVar7;
  char *_Str1;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 auStack_18 [3];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9901;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar3 = operator_new(0x10d0);
  puVar7 = (undefined4 *)0x0;
  local_4 = 0;
  if (pvVar3 == (void *)0x0) {
    pvVar3 = (void *)0x0;
  }
  else {
    pvVar3 = MeshWorld_ctor(pvVar3,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\level9");
  }
  local_4 = 0xffffffff;
  param_1[0x22b] = (int)pvVar3;
  pvVar3 = operator_new(0x10d0);
  local_4 = 1;
  if (pvVar3 == (void *)0x0) {
    pvVar3 = (void *)0x0;
  }
  else {
    pvVar3 = CollisionLevel_ctorWithLevel(pvVar3,param_1[0x22b]);
  }
  local_4 = 0xffffffff;
  param_1[0x22c] = (int)pvVar3;
  Level_InitScene((int)param_1);
  (**(code **)(*param_1 + 0x80))();
  iVar4 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar5 = *(int *)(param_1[0x22b] + 0x480);
  *(undefined4 *)(iVar5 + 0x89c + iVar4 * 4) = 0;
  if (0 < *(int *)(iVar5 + 0x898)) {
    puVar7 = (undefined4 *)**(undefined4 **)(iVar5 + 0xca0);
    *(undefined4 *)(iVar5 + 0x89c + iVar4 * 4) = 1;
  }
  while (puVar7 != (undefined4 *)0x0) {
    iVar5 = __strnicmp((char *)*puVar7,"PILLAR",6);
    if (iVar5 == 0) {
      AthenaList_Append(param_1 + 0x10e5,(int)puVar7);
    }
    _Str1 = (char *)*puVar7;
    iVar5 = __strnicmp(_Str1,"MAGNIFYER",9);
    if ((iVar5 == 0) && (*(int *)(param_1[0x21e] + 0x23c) != 0)) {
      uVar9 = 0x444;
      uVar8 = 0x410979;
      pvVar3 = operator_new(0x444);
      local_4 = 2;
      if (pvVar3 == (void *)0x0) {
        pvVar3 = (void *)0x0;
      }
      else {
        if ((undefined4 *)&stack0xffffffc8 != puVar7 + 1) {
          _Str1 = (char *)puVar7[1];
          uVar8 = puVar7[2];
          uVar9 = puVar7[3];
        }
        pvVar3 = CollisionLevel_Spatial_Ctor(pvVar3,param_1,_Str1,uVar8,uVar9);
      }
      local_4 = 0xffffffff;
      param_1[0x11eb] = (int)pvVar3;
    }
    iVar5 = *(int *)(param_1[0x22b] + 0x480);
    iVar2 = *(int *)(iVar5 + 0x89c + iVar4 * 4);
    if (*(int *)(iVar5 + 0x898) <= iVar2) break;
    puVar7 = *(undefined4 **)(*(int *)(iVar5 + 0xca0) + iVar2 * 4);
    *(int *)(iVar5 + 0x89c + iVar4 * 4) = iVar2 + 1;
  }
  piVar6 = (int *)AthenaHashTable_Lookup
                            ((void *)param_1[0x22b],auStack_18,"CLOUDSCAPE",(undefined1 *)0x0);
  if (param_1 + 0x10de != piVar6) {
    param_1[0x10de] = *piVar6;
    param_1[0x10df] = piVar6[1];
    param_1[0x10e0] = piVar6[2];
  }
  pvVar3 = *(void **)(param_1[0x21e] + 0x174);
  Graphics_SetViewportZ
            (pvVar3,(*(float *)((int)pvVar3 + 0x188) * _DAT_004cf6f4 + _DAT_004cf6f4) *
                    (float)_DAT_004cf3e0,
             ((*(float *)((int)pvVar3 + 0x188) + *(float *)((int)pvVar3 + 0x188)) * _DAT_004cf6fc +
             _DAT_004cf6f8) * (float)_DAT_004cf3e0);
  fVar1 = *(float *)((int)*(void **)(param_1[0x21e] + 0x174) + 0x188);
  Graphics_SetProjection
            (*(void **)(param_1[0x21e] + 0x174),20.0,
             ((fVar1 + fVar1) * _DAT_004cf6fc + _DAT_004cf6f8) * (float)_DAT_004cf3e0);
  ExceptionList = pvStack_c;
  return;
}
