/*
 * Function: Scene_LoadLevel3
 * Address: 0x0040d390
 * Signature: void __fastcall Scene_LoadLevel3(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_LoadLevel3, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaList_NextIndex, __strnicmp, AthenaList_Append. Offsets: 8, Lines: 65
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_LoadLevel3(int *param_1)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
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
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\level3");
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
  iVar3 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar4 = *(int *)(param_1[0x22b] + 0x480);
  *(undefined4 *)(iVar4 + 0x89c + iVar3 * 4) = 0;
  if (*(int *)(iVar4 + 0x898) < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = (undefined4 *)**(undefined4 **)(iVar4 + 0xca0);
    *(undefined4 *)(iVar4 + 0x89c + iVar3 * 4) = 1;
  }
  while( true ) {
    if (puVar5 == (undefined4 *)0x0) {
      ExceptionList = pvStack_c;
      return;
    }
    iVar4 = __strnicmp((char *)*puVar5,"TarBubble",9);
    if (iVar4 == 0) {
      AthenaList_Append(param_1 + 0x11e4,(int)puVar5);
    }
    iVar4 = *(int *)(param_1[0x22b] + 0x480);
    iVar1 = *(int *)(iVar4 + 0x89c + iVar3 * 4);
    if (*(int *)(iVar4 + 0x898) <= iVar1) break;
    puVar5 = *(undefined4 **)(*(int *)(iVar4 + 0xca0) + iVar1 * 4);
    *(int *)(iVar4 + 0x89c + iVar3 * 4) = iVar1 + 1;
  }
  ExceptionList = pvStack_c;
  return;
}
