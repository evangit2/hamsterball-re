/*
 * Function: BoardLevel_Up_Dtor
 * Address: 0x00420550
 * Signature: void __fastcall BoardLevel_Up_Dtor(undefined4 *param_1)
 *
 * Patterns: frees memory, SEH frame, collision, scene, board, level. Calls: BoardLevel_Up_Dtor, AthenaList_NextIndex, CollisionFace_dtor_vtable, _free, AthenaList_Free, Vec3List_Free, Scene_dtor. Offsets: 2, Lines: 53
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BoardLevel_Up_Dtor(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *_Memory;
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004ca6a6;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_BoardLevel_Generic_dtor3_004d11a0;
  puVar1 = param_1 + 0x10db;
  local_4 = 1;
  iVar3 = AthenaList_NextIndex((int)puVar1);
  param_1[iVar3 + 0x10dd] = 0;
  if ((int)param_1[0x10dc] < 1) {
    _Memory = (undefined4 *)0x0;
  }
  else {
    _Memory = *(undefined4 **)param_1[0x11de];
    param_1[iVar3 + 0x10dd] = 1;
  }
  while (_Memory != (undefined4 *)0x0) {
    CollisionFace_dtor_vtable(_Memory);
    _free(_Memory);
    iVar2 = param_1[iVar3 + 0x10dd];
    if ((int)param_1[0x10dc] <= iVar2) break;
    _Memory = *(undefined4 **)(param_1[0x11de] + iVar2 * 4);
    param_1[iVar3 + 0x10dd] = iVar2 + 1;
  }
  AthenaList_Free((int)puVar1);
  if ((undefined4 *)param_1[0x11e1] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11e1])(1);
  }
  param_1[0x11e1] = 0;
  if ((undefined4 *)param_1[0x11e2] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11e2])(1);
  }
  param_1[0x11e2] = 0;
  if ((undefined4 *)param_1[0x11e3] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x11e3])(1);
  }
  param_1[0x11e3] = 0;
  local_4 = local_4 & 0xffffff00;
  Vec3List_Free(puVar1);
  local_4 = 0xffffffff;
  Scene_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
