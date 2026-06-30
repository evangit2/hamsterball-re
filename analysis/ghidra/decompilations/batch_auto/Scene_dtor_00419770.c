/*
 * Function: Scene_dtor
 * Address: 0x00419770
 * Signature: Scene_dtor(...)
 *
 * Patterns: vtable dispatch, SEH frame, matrix math, rendering, ball, scene, board. Calls: x335, x43B, x22E, Scene_dtor, AthenaList_NextIndex, thunk_Gfx_SetRenderState, AthenaList_Free, NoOp_return. Offsets: 24, Lines: 175
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Scene_dtor - destroys Scene object. Cleans up: vtable@0xD87 callback, Ball list@0x335
   (array@0x438), effect list@0x43B (array@0x53E), object list@0x22E (array@0x331), vtable
   dtors@0x22C/0x22B/0x21F/0xE92, lists@0xA75/0xC81, vtable@0xFC6. Also cleans:
   Vec3Lists@0xD8B/0xB7B/0x95E/0x858/0x752/0x64C/0x546/0x43B/0x335/0x22E, RumbleTimers@0x226/0x221,
   SceneObject base@0xEBF */

void __fastcall Scene_dtor(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004c9fd8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_Scene_DeletingDtor_004d0260;
  puVar5 = (undefined4 *)0x0;
  local_4 = 0x12;
  if ((undefined4 *)param_1[0xd87] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xd87])(1);
  }
  iVar2 = AthenaList_NextIndex((int)(param_1 + 0x335));
  param_1[iVar2 + 0x337] = 0;
  if ((int)param_1[0x336] < 1) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3 = *(undefined4 **)param_1[0x438];
    param_1[iVar2 + 0x337] = 1;
  }
  while (puVar3 != (undefined4 *)0x0) {
    (**(code **)*puVar3)(1);
    iVar1 = param_1[iVar2 + 0x337];
    if ((int)param_1[0x336] <= iVar1) break;
    puVar3 = *(undefined4 **)(param_1[0x438] + iVar1 * 4);
    param_1[iVar2 + 0x337] = iVar1 + 1;
  }
  iVar2 = AthenaList_NextIndex((int)(param_1 + 0x43b));
  param_1[iVar2 + 0x43d] = 0;
  if (0 < (int)param_1[0x43c]) {
    puVar5 = *(undefined4 **)param_1[0x53e];
    param_1[iVar2 + 0x43d] = 1;
  }
  while (puVar5 != (undefined4 *)0x0) {
    thunk_Gfx_SetRenderState((void *)(param_1[0x22c] + 0x18),(int)puVar5);
    (**(code **)*puVar5)(1);
    iVar1 = param_1[iVar2 + 0x43d];
    if ((int)param_1[0x43c] <= iVar1) break;
    puVar5 = *(undefined4 **)(param_1[0x53e] + iVar1 * 4);
    param_1[iVar2 + 0x43d] = iVar1 + 1;
  }
  iVar2 = AthenaList_NextIndex((int)(param_1 + 0x22e));
  param_1[iVar2 + 0x230] = 0;
  if ((int)param_1[0x22f] < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = *(undefined4 **)param_1[0x331];
    param_1[iVar2 + 0x230] = 1;
  }
  while (puVar5 != (undefined4 *)0x0) {
    (**(code **)*puVar5)(1);
    iVar1 = param_1[iVar2 + 0x230];
    if ((int)param_1[0x22f] <= iVar1) break;
    puVar5 = *(undefined4 **)(param_1[0x331] + iVar1 * 4);
    param_1[iVar2 + 0x230] = iVar1 + 1;
  }
  AthenaList_Free((int)(param_1 + 0x22e));
  if ((undefined4 *)param_1[0x22c] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x22c])(1);
    param_1[0x22c] = 0;
  }
  if ((undefined4 *)param_1[0x22b] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x22b])(1);
    param_1[0x22b] = 0;
  }
  if ((undefined4 *)param_1[0x21f] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x21f])(1);
    param_1[0x21f] = 0;
  }
  if ((undefined4 *)param_1[0xe92] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xe92])(1);
    param_1[0xe92] = 0;
  }
  puVar5 = param_1 + 0xa75;
  iVar2 = AthenaList_NextIndex((int)puVar5);
  param_1[iVar2 + 0xa77] = 0;
  if ((int)param_1[0xa76] < 1) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3 = *(undefined4 **)param_1[0xb78];
    param_1[iVar2 + 0xa77] = 1;
  }
  while (puVar3 != (undefined4 *)0x0) {
    (**(code **)*puVar3)(1);
    iVar1 = param_1[iVar2 + 0xa77];
    if ((int)param_1[0xa76] <= iVar1) break;
    puVar3 = *(undefined4 **)(param_1[0xb78] + iVar1 * 4);
    param_1[iVar2 + 0xa77] = iVar1 + 1;
  }
  puVar3 = param_1 + 0xc81;
  iVar2 = AthenaList_NextIndex((int)puVar3);
  param_1[iVar2 + 0xc83] = 0;
  if ((int)param_1[0xc82] < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = *(undefined4 **)param_1[0xd84];
    param_1[iVar2 + 0xc83] = 1;
  }
  while (puVar4 != (undefined4 *)0x0) {
    (**(code **)*puVar4)(1);
    iVar1 = param_1[iVar2 + 0xc83];
    if ((int)param_1[0xc82] <= iVar1) break;
    puVar4 = *(undefined4 **)(param_1[0xd84] + iVar1 * 4);
    param_1[iVar2 + 0xc83] = iVar1 + 1;
  }
  AthenaList_Free((int)puVar5);
  AthenaList_Free((int)puVar3);
  AthenaList_Free((int)(param_1 + 0x858));
  AthenaList_Free((int)(param_1 + 0x64c));
  AthenaList_Free((int)(param_1 + 0x335));
  AthenaList_Free((int)(param_1 + 0x43b));
  if ((undefined4 *)param_1[0xfc6] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0xfc6])(1);
  }
  param_1[0xfc6] = 0;
  NoOp_return();
  local_4._0_1_ = 0x11;
  _eh_vector_destructor_iterator_(param_1 + 0xfc8,0x10,2,NoOp);
  local_4._0_1_ = 0x10;
  SceneObject_BaseDtor(param_1 + 0xebf);
  local_4._0_1_ = 0xf;
  _eh_vector_destructor_iterator_(param_1 + 0xeab,0x14,4,Matrix_Identity);
  local_4._0_1_ = 0xe;
  Vec3List_Free(param_1 + 0xd8b);
  local_4._0_1_ = 0xd;
  Vec3List_Free(puVar3);
  local_4._0_1_ = 0xc;
  Vec3List_Free(param_1 + 0xb7b);
  local_4._0_1_ = 0xb;
  Vec3List_Free(puVar5);
  local_4._0_1_ = 10;
  Vec3List_Free(param_1 + 0x95e);
  local_4._0_1_ = 9;
  Vec3List_Free(param_1 + 0x858);
  local_4._0_1_ = 8;
  Vec3List_Free(param_1 + 0x752);
  local_4._0_1_ = 7;
  Vec3List_Free(param_1 + 0x64c);
  local_4._0_1_ = 6;
  Vec3List_Free(param_1 + 0x546);
  local_4._0_1_ = 5;
  Matrix_Identity(param_1 + 0x541);
  local_4._0_1_ = 4;
  Vec3List_Free(param_1 + 0x43b);
  local_4._0_1_ = 3;
  Vec3List_Free(param_1 + 0x335);
  local_4._0_1_ = 2;
  Vec3List_Free(param_1 + 0x22e);
  local_4._0_1_ = 1;
  ToggleTimer_Cleanup(param_1 + 0x226);
  local_4 = (uint)local_4._1_3_ << 8;
  ToggleTimer_Cleanup(param_1 + 0x221);
  local_4 = 0xffffffff;
  SceneObject_dtor(param_1);
  ExceptionList = pvStack_c;
  return;
}
