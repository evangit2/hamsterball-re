/*
 * Function: Scene_ProcessRaceEnd
 * Address: 0x0041a540
 * Signature: Scene_ProcessRaceEnd(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, audio, scene, menu. Calls: Scene_ProcessRaceEnd, AthenaList_NextIndex, AthenaList_Free, operator_new, TourneyMenu_ctor, Scene_AddObject, MusicPlayer_SetTempoScale, App_ShowResults. Offsets: 22, Lines: 177
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_ProcessRaceEnd(int *param_1)

{
  int iVar1;
  void *this;
  float fVar2;
  bool bVar3;
  int iVar4;
  void *pvVar5;
  int *piVar6;
  undefined4 *puVar7;
  int iVar8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca047;
  local_c = ExceptionList;
  bVar3 = false;
  ExceptionList = &local_c;
  iVar4 = AthenaList_NextIndex((int)(param_1 + 0xd8b));
  param_1[iVar4 + 0xd8d] = 0;
  if (param_1[0xd8c] < 1) {
    iVar8 = 0;
  }
  else {
    iVar8 = *(int *)param_1[0xe8e];
    param_1[iVar4 + 0xd8d] = 1;
  }
  if (iVar8 != 0) {
    do {
      if ((*(char *)(iVar8 + 10) == '\0') ||
         ((**(code **)(*param_1 + 0x5c))(iVar8), *(int *)(iVar8 + 0x30) != 5)) {
        bVar3 = true;
      }
      iVar1 = param_1[iVar4 + 0xd8d];
      if (param_1[0xd8c] <= iVar1) break;
      iVar8 = *(int *)(param_1[0xe8e] + iVar1 * 4);
      param_1[iVar4 + 0xd8d] = iVar1 + 1;
    } while (iVar8 != 0);
    if (bVar3) {
      ExceptionList = local_c;
      return;
    }
  }
  if (param_1[0xd8a] < 1) {
    ExceptionList = local_c;
    return;
  }
  if ((((char)param_1[0x10d2] != '\0') &&
      (*(char *)(*(int *)(param_1[0x21e] + 0x220) + 0x10) == '\0')) &&
     (*(char *)(*(int *)(param_1[0x21e] + 0x220) + 0x11) == '\0')) {
    *(undefined1 *)(param_1 + 0x21d) = 1;
    iVar4 = AthenaList_NextIndex((int)(param_1 + 0x22e));
    param_1[iVar4 + 0x230] = 0;
    if (param_1[0x22f] < 1) {
      puVar7 = (undefined4 *)0x0;
    }
    else {
      puVar7 = *(undefined4 **)param_1[0x331];
      param_1[iVar4 + 0x230] = 1;
    }
    while (puVar7 != (undefined4 *)0x0) {
      (**(code **)*puVar7)(1);
      iVar8 = param_1[iVar4 + 0x230];
      if (param_1[0x22f] <= iVar8) break;
      puVar7 = *(undefined4 **)(param_1[0x331] + iVar8 * 4);
      param_1[iVar4 + 0x230] = iVar8 + 1;
    }
    AthenaList_Free((int)(param_1 + 0x22e));
    pvVar5 = operator_new(0xcfc);
    local_4 = 4;
    if (pvVar5 != (void *)0x0) {
      piVar6 = TourneyMenu_ctor(pvVar5,param_1[0x21e]);
      goto LAB_0041a84a;
    }
LAB_0041a848:
    piVar6 = (int *)0x0;
LAB_0041a84a:
    local_4 = 0xffffffff;
    Scene_AddObject(*(void **)(param_1[0x21e] + 0x184),piVar6);
    ExceptionList = local_c;
    return;
  }
  if (((float)param_1[0xd89] == _DAT_004cf368) &&
     (*(void **)(param_1[0x21e] + 0x53c) != (void *)0x0)) {
    MusicPlayer_SetTempoScale(*(void **)(param_1[0x21e] + 0x53c),2.0);
  }
  fVar2 = (float)param_1[0xd89] + _DAT_004cf520;
  param_1[0xd89] = (int)fVar2;
  if (*(char *)(*(int *)(param_1[0x21e] + 0x220) + 0x10) != '\0') {
    param_1[0xd89] = (int)(fVar2 + _DAT_004cf524);
  }
  if ((float)param_1[0xd89] < _DAT_004cf310) {
    ExceptionList = local_c;
    return;
  }
  iVar4 = AthenaList_NextIndex((int)(param_1 + 0x22e));
  param_1[iVar4 + 0x230] = 0;
  if (param_1[0x22f] < 1) {
    puVar7 = (undefined4 *)0x0;
  }
  else {
    puVar7 = *(undefined4 **)param_1[0x331];
    param_1[iVar4 + 0x230] = 1;
  }
  while (puVar7 != (undefined4 *)0x0) {
    (**(code **)*puVar7)(1);
    iVar8 = param_1[iVar4 + 0x230];
    if (param_1[0x22f] <= iVar8) break;
    puVar7 = *(undefined4 **)(param_1[0x331] + iVar8 * 4);
    param_1[iVar4 + 0x230] = iVar8 + 1;
  }
  AthenaList_Free((int)(param_1 + 0x22e));
  *(undefined4 *)(param_1[0x21e] + 0x5ec) = *(undefined4 *)(param_1[0x21e] + 0x5e8);
  *(undefined4 *)(param_1[0x21e] + 0x68c) = *(undefined4 *)(param_1[0x21e] + 0x688);
  *(undefined4 *)(param_1[0x21e] + 0x72c) = *(undefined4 *)(param_1[0x21e] + 0x728);
  *(undefined4 *)(param_1[0x21e] + 0x7cc) = *(undefined4 *)(param_1[0x21e] + 0x7c8);
  pvVar5 = (void *)param_1[0x21e];
  this = *(void **)((int)pvVar5 + 0x220);
  if (*(char *)((int)this + 0x11) == '\0') {
    if (*(char *)((int)this + 0x10) != '\0') {
      App_ShowResults(pvVar5,'\0');
      pvVar5 = operator_new(0xd18);
      local_4 = 3;
      if (pvVar5 == (void *)0x0) {
        piVar6 = (int *)0x0;
      }
      else {
        piVar6 = PracticeMenu_ctor(pvVar5,param_1[0x21e]);
      }
      local_4 = 0xffffffff;
      Scene_AddObject(*(void **)(param_1[0x21e] + 0x184),piVar6);
      App_StartRace(param_1[0x21e]);
      ExceptionList = local_c;
      return;
    }
    Tournament_AdvanceRace(this,'\x01');
    ExceptionList = local_c;
    return;
  }
  App_StartRace((int)pvVar5);
  App_ShowResults((void *)param_1[0x21e],'\0');
  if (*(char *)(param_1[0x21e] + 0x234) == '\0') {
    pvVar5 = operator_new(0xd18);
    local_4 = 2;
    if (pvVar5 != (void *)0x0) {
      piVar6 = TimeTrialMenu_ctor(pvVar5,param_1[0x21e]);
      goto LAB_0041a8a6;
    }
  }
  else {
    if (*(char *)(param_1[0x21e] + 0x237) != '\0') {
      pvVar5 = operator_new(0xcdc);
      local_4 = 0;
      if (pvVar5 != (void *)0x0) {
        piVar6 = ArenaMenu_ctor(pvVar5,param_1[0x21e]);
        goto LAB_0041a84a;
      }
      goto LAB_0041a848;
    }
    pvVar5 = operator_new(0xd18);
    local_4 = 1;
    if (pvVar5 != (void *)0x0) {
      piVar6 = PartyMenu_ctor(pvVar5,param_1[0x21e]);
      goto LAB_0041a8a6;
    }
  }
  piVar6 = (int *)0x0;
LAB_0041a8a6:
  local_4 = 0xffffffff;
  Scene_AddObject(*(void **)(param_1[0x21e] + 0x184),piVar6);
  ExceptionList = local_c;
  return;
}
