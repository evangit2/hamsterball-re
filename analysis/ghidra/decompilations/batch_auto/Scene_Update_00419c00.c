/*
 * Function: Scene_Update
 * Address: 0x00419c00
 * Signature: Scene_Update(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, rendering, input, ball, scene, board, menu. Calls: Scene_Update, operator_new, ScoreDisplay_CtorC, Scene_AddObject, AthenaList_GetSize, Input_CheckKeyCombo, Scene_CreateGameOverMenu, AthenaList_NextIndex. Offsets: 26, Lines: 172
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Scene_Update(this): Main game tick. 1)frame++ 2)demo timer 3)ESC check 4)ball position
   propagation 5)gear path follow 6)RumbleBoard timer ticks 7)camera shake decay 8)scene object
   update+render vtable[4]/vtable[0] 9)physics pipeline vtable[4C/50/54/58]+physics objects 10)post
   callback. See decomp_scene_update.c */

void __fastcall Scene_Update(int *param_1)

{
  int iVar1;
  void *pvVar2;
  int *piVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  bool bVar8;
  bool bVar9;
  float in_stack_ffffffd4;
  float in_stack_ffffffd8;
  float fVar10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9ffb;
  local_c = ExceptionList;
  piVar7 = (int *)0x0;
  ExceptionList = &local_c;
  param_1[0xd88] = param_1[0xd88] + 1;
  if (param_1[0x10d6] != 0) {
    iVar6 = param_1[0x10d7] + -1;
    param_1[0x10d7] = iVar6;
    if (iVar6 < 1) {
      if (iVar6 == 0) {
        param_1[0x10d8] = 0;
        param_1[0x10d9] = 0;
      }
      if (_DAT_004d039c <= (float)param_1[0x10d8]) {
        *(undefined1 *)(param_1 + 0x21d) = 1;
        pvVar2 = operator_new(0x1160);
        local_4 = 4;
        if (pvVar2 == (void *)0x0) {
          piVar3 = (int *)0x0;
        }
        else {
          in_stack_ffffffd4 = (float)param_1[0x21e];
          piVar3 = ScoreDisplay_CtorC(pvVar2,(int)in_stack_ffffffd4,
                                      (byte *)
                                      "You have reached the end of the demo version of Hamsterball!  But, if you buy now, you can continue, right here, right now!  Or, click cancel to return to the main menu."
                                      ,param_1);
        }
        local_4 = 0xffffffff;
        in_stack_ffffffd8 = 6.025548e-39;
        Scene_AddObject(*(void **)(param_1[0x21e] + 0x184),piVar3);
        param_1[0x10d6] = 0;
      }
      iVar6 = param_1[0x10d9];
      param_1[0x10d9] = iVar6 + 1;
      if ((float)(iVar6 + 1) < (float)param_1[0x10d8]) {
        ExceptionList = local_c;
        return;
      }
      param_1[0x10d9] = 0;
      param_1[0x10d8] = (int)((float)param_1[0x10d8] + (float)_DAT_004cf3e0);
    }
  }
  if (((char)param_1[0x10da] == '\0') ||
     (iVar6 = AthenaList_GetSize(*(int *)(param_1[0x21e] + 0x184) + 4), iVar6 < 2)) {
    iVar6 = *(int *)(param_1[0x21e] + 0x5fc);
    if (((iVar6 < 3) || (4 < iVar6)) && ((char)param_1[0x220] == '\0')) {
      in_stack_ffffffd8 = 6.025753e-39;
      uVar4 = Input_CheckKeyCombo((void *)param_1[0x21e],2);
      if ((char)uVar4 != '\0') {
        in_stack_ffffffd8 = 6.025771e-39;
        Scene_CreateGameOverMenu(param_1,'\x01');
      }
    }
    if ((char)param_1[0xa6c] != '\0') {
      iVar6 = AthenaList_NextIndex((int)(param_1 + 0xa75));
      param_1[iVar6 + 0xa77] = 0;
      if (param_1[0xa76] < 1) {
        pvVar2 = (void *)0x0;
      }
      else {
        pvVar2 = *(void **)param_1[0xb78];
        param_1[iVar6 + 0xa77] = 1;
      }
      while (pvVar2 != (void *)0x0) {
        in_stack_ffffffd8 = *(float *)((int)pvVar2 + 0x168);
        in_stack_ffffffd4 = *(float *)((int)pvVar2 + 0x164);
        Ball_SetTargetPos(pvVar2,in_stack_ffffffd4,in_stack_ffffffd8,*(float *)((int)pvVar2 + 0x16c)
                         );
        iVar5 = param_1[iVar6 + 0xa77];
        if (param_1[0xa76] <= iVar5) break;
        pvVar2 = *(void **)(param_1[0xb78] + iVar5 * 4);
        param_1[iVar6 + 0xa77] = iVar5 + 1;
      }
      *(undefined1 *)(param_1 + 0xa6c) = 0;
    }
    piVar3 = param_1 + 0xd8b;
    fVar10 = 6.025974e-39;
    iVar6 = AthenaList_GetSize((int)piVar3);
    if ((iVar6 == 1) && (param_1[0xfc7] != 0)) {
      iVar6 = *(int *)(param_1[0x21e] + 0x5dc);
      if ((float *)&stack0xffffffd4 != (float *)(iVar6 + 0x758)) {
        in_stack_ffffffd4 = *(float *)(iVar6 + 0x758);
        in_stack_ffffffd8 = *(float *)(iVar6 + 0x75c);
        fVar10 = *(float *)(iVar6 + 0x760);
      }
      Gear_AdvanceAlongPath(param_1 + 0xfc8,in_stack_ffffffd4,in_stack_ffffffd8,fVar10);
    }
    ToggleTimer_Tick((int)(param_1 + 0x221));
    ToggleTimer_Tick((int)(param_1 + 0x226));
    bVar8 = (char)param_1[0xe93] != '\0';
    iVar6 = param_1[0xa6e];
    if (bVar8) {
      iVar5 = 800;
      bVar9 = iVar6 == -800;
    }
    else {
      iVar5 = 0;
      bVar9 = iVar6 == 0;
    }
    if (!bVar9 && (bVar8 && SBORROW4(iVar6,-800)) == iVar6 + iVar5 < 0) {
      param_1[0xa6e] = iVar6 + -10;
    }
    iVar6 = AthenaList_NextIndex((int)(param_1 + 0x22e));
    param_1[iVar6 + 0x230] = 0;
    if (0 < param_1[0x22f]) {
      piVar7 = *(int **)param_1[0x331];
      param_1[iVar6 + 0x230] = 1;
    }
    while (piVar7 != (int *)0x0) {
      (**(code **)(*piVar7 + 4))();
      if ((char)piVar7[6] != '\0') {
        thunk_Gfx_SetRenderState(param_1 + 0x22e,(int)piVar7);
        (**(code **)*piVar7)();
      }
      iVar5 = param_1[iVar6 + 0x230];
      if (param_1[0x22f] <= iVar5) break;
      piVar7 = *(int **)(param_1[0x331] + iVar5 * 4);
      param_1[iVar6 + 0x230] = iVar5 + 1;
    }
    iVar6 = AthenaList_GetSize((int)piVar3);
    if ((iVar6 != 1) || (*(char *)(*(int *)(param_1[0x21e] + 0x5dc) + 0x14c) == '\0')) {
      (**(code **)(*param_1 + 0x4c))();
      (**(code **)(*param_1 + 0x50))();
      (**(code **)(*param_1 + 0x54))();
      (**(code **)(*param_1 + 0x58))();
      iVar6 = AthenaList_NextIndex((int)piVar3);
      param_1[iVar6 + 0xd8d] = 0;
      if (param_1[0xd8c] < 1) {
        iVar5 = 0;
      }
      else {
        iVar5 = *(int *)param_1[0xe8e];
        param_1[iVar6 + 0xd8d] = 1;
      }
      while (iVar5 != 0) {
        (**(code **)(*param_1 + 0x7c))();
        iVar1 = param_1[iVar6 + 0xd8d];
        if (param_1[0xd8c] <= iVar1) break;
        iVar5 = *(int *)(param_1[0xe8e] + iVar1 * 4);
        param_1[iVar6 + 0xd8d] = iVar1 + 1;
      }
      (**(code **)(param_1[0xebf] + 4))();
    }
  }
  ExceptionList = local_c;
  return;
}
