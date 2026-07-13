/*
 * Function: Scene_LoadLevel4
 * Address: 0x0040d6d0
 * Signature: void __fastcall Scene_LoadLevel4(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_LoadLevel4, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, FlagWaver_AllocBuffers. Offsets: 4, Lines: 42
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_LoadLevel4(int *param_1)

{
  int iVar1;
  void *pvVar2;
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
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\level4");
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
  *(undefined1 *)(param_1[0xfc6] + 0x80) = 1;
  iVar1 = param_1[0xfc6];
  *(undefined4 *)(iVar1 + 0x10) = 0x42fa0000;
  *(undefined4 *)(iVar1 + 0x14) = 0x42960000;
  FlagWaver_AllocBuffers(param_1[0xfc6]);
  ExceptionList = pvStack_c;
  return;
}
