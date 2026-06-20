/*
 * Function: LevelGlass_InitScene
 * Address: 0x00417640
 * Signature: void __fastcall LevelGlass_InitScene(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: LevelGlass_InitScene, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaList_NextIndex. Offsets: 5, Lines: 59
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall LevelGlass_InitScene(int *param_1)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
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
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\levelglass");
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
  iVar3 = AthenaList_NextIndex((int)(param_1 + 0xa75));
  param_1[iVar3 + 0xa77] = 0;
  if (param_1[0xa76] < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = *(int *)param_1[0xb78];
    param_1[iVar3 + 0xa77] = 1;
  }
  while( true ) {
    if (iVar4 == 0) {
      ExceptionList = pvStack_c;
      return;
    }
    *(undefined1 *)(iVar4 + 0x280) = 1;
    iVar1 = param_1[iVar3 + 0xa77];
    if (param_1[0xa76] <= iVar1) break;
    iVar4 = *(int *)(param_1[0xb78] + iVar1 * 4);
    param_1[iVar3 + 0xa77] = iVar1 + 1;
  }
  ExceptionList = pvStack_c;
  return;
}
