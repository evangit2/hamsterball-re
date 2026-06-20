/*
 * Function: Scene_SetupLevel6
 * Address: 0x0040ea90
 * Signature: void __fastcall Scene_SetupLevel6(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, scene, level. Calls: Scene_SetupLevel6, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, Level_InitScene, AthenaHashTable_Lookup. Offsets: 7, Lines: 71
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_SetupLevel6(int *param_1)

{
  void *pvVar1;
  int *piVar2;
  undefined4 auStack_24 [3];
  undefined4 auStack_18 [3];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c97a6;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar1 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\level6");
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
  *(undefined1 *)(param_1 + 0x10dc) = 0;
  param_1[0x10e8] = -1;
  piVar2 = (int *)AthenaHashTable_Lookup
                            ((void *)param_1[0x22b],auStack_24,"LAUNCH01",(undefined1 *)0x0);
  if (param_1 + 0x10df != piVar2) {
    param_1[0x10df] = *piVar2;
    param_1[0x10e0] = piVar2[1];
    param_1[0x10e1] = piVar2[2];
  }
  piVar2 = (int *)AthenaHashTable_Lookup
                            ((void *)param_1[0x22b],auStack_24,"LAUNCH02",(undefined1 *)0x0);
  if (param_1 + 0x10e2 != piVar2) {
    param_1[0x10e2] = *piVar2;
    param_1[0x10e3] = piVar2[1];
    param_1[0x10e4] = piVar2[2];
  }
  piVar2 = (int *)AthenaHashTable_Lookup
                            ((void *)param_1[0x22b],auStack_24,"LAUNCH03",(undefined1 *)0x0);
  if (param_1 + 0x10e5 != piVar2) {
    param_1[0x10e5] = *piVar2;
    param_1[0x10e6] = piVar2[1];
    param_1[0x10e7] = piVar2[2];
  }
  piVar2 = (int *)AthenaHashTable_Lookup
                            ((void *)param_1[0x22b],auStack_18,"CHROMESHADOW",(undefined1 *)0x0);
  if (param_1 + 0x10e9 != piVar2) {
    param_1[0x10e9] = *piVar2;
    param_1[0x10ea] = piVar2[1];
    param_1[0x10eb] = piVar2[2];
  }
  param_1[0x10de] = 0;
  param_1[0x10dd] = 200;
  ExceptionList = pvStack_c;
  return;
}
