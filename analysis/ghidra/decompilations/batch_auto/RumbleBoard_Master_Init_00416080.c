/*
 * Function: RumbleBoard_Master_Init
 * Address: 0x00416080
 * Signature: void __fastcall RumbleBoard_Master_Init(int *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, collision, camera, board, level. Calls: RumbleBoard_Master_Init, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, CameraLookAt. Offsets: 2, Lines: 36
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall RumbleBoard_Master_Init(int *param_1)

{
  void *pvVar1;
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
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(param_1[0x21e] + 0x174),"levels\\arena-Master");
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
  CameraLookAt(param_1);
  (**(code **)(*param_1 + 0x80))();
  ExceptionList = pvStack_c;
  return;
}
