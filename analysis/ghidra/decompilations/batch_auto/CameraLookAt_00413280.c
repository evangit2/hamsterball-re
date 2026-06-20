/*
 * Function: CameraLookAt
 * Address: 0x00413280
 * Signature: CameraLookAt(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, camera, scene, level. Calls: params, CameraLookAt, operator_new, MeshWorld_ctor, Level_InitScene, AthenaHashTable_Lookup. Offsets: 3, Lines: 64
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* CameraLookAt: Arena camera init. Loads arena-spawnplatform + arena-stands meshes, finds
   CAMERALOOKAT target, sets camera params (distance=45, height=800, max_height=800). See
   decompilations/scene/decomp_camera_system.c */

void __fastcall CameraLookAt(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *pvVar4;
  int *piVar5;
  undefined4 auStack_20 [3];
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c97a6;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  pvVar4 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = MeshWorld_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174),
                            "levels\\arena-spawnplatform");
  }
  local_4 = 0xffffffff;
  param_1[0x10e3] = (int)pvVar4;
  pvVar4 = operator_new(0x10d0);
  local_4 = 1;
  if (pvVar4 == (void *)0x0) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = MeshWorld_ctor(pvVar4,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\arena-stands");
  }
  param_1[0x10e4] = (int)pvVar4;
  local_4 = 0xffffffff;
  (**(code **)(*param_1 + 0x90))(param_1[0x10e3]);
  (**(code **)(*param_1 + 0x90))(param_1[0x10e4]);
  Level_InitScene((int)param_1);
  piVar5 = (int *)AthenaHashTable_Lookup
                            ((void *)param_1[0x22b],auStack_20,"CAMERALOOKAT",(undefined1 *)0x0);
  iVar1 = piVar5[1];
  iVar3 = *piVar5;
  iVar2 = piVar5[2];
  param_1[0x10de] = iVar3;
  param_1[0x10df] = iVar1;
  param_1[0x10e0] = iVar2;
  param_1[0x10db] = iVar3;
  param_1[0x10dc] = iVar1;
  param_1[0x10dd] = iVar2;
  param_1[0xa6f] = 0x42340000;
  param_1[0xa70] = 0x44480000;
  param_1[0x10e1] = 0x44480000;
  param_1[0x10e2] = 1;
  (**(code **)(*param_1 + 0x54))();
  ExceptionList = pvStack_14;
  return;
}
