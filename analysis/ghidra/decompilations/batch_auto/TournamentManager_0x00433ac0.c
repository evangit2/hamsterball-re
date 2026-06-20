
void __thiscall TournamentManager(void *this,char *param_1)

{
  char cVar1;
  void *pvVar2;
  int *piVar3;
  int iVar4;
  char *pcVar5;
  char *pcVar6;
  bool bVar7;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  pvVar2 = ExceptionList;
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cb6a3;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined1 *)(*(int *)((int)this + 0x878) + 0x234) = 0;
  if (param_1 == (char *)0x0) {
    ExceptionList = pvVar2;
    return;
  }
  iVar4 = 5;
  bVar7 = true;
  pcVar5 = param_1;
  pcVar6 = "BACK";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *pcVar5 == *pcVar6;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  } while (bVar7);
  if (bVar7) {
    (**(code **)(*(int *)this + 0x40))();
    pvVar2 = operator_new(0xcdc);
    uStack_4 = 0;
    if (pvVar2 == (void *)0x0) {
      pvVar2 = (void *)0x0;
    }
    else {
      pvVar2 = MainMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
    }
    *(void **)(*(int *)((int)this + 0x878) + 0x224) = pvVar2;
    uStack_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),
                    *(int **)(*(int *)((int)this + 0x878) + 0x224));
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x184) + 0x420) =
         *(undefined4 *)(*(int *)((int)this + 0x878) + 0x224);
  }
  iVar4 = 7;
  bVar7 = true;
  pcVar5 = param_1;
  pcVar6 = "LOCKED";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *pcVar5 == *pcVar6;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  } while (bVar7);
  if (bVar7) {
    pvVar2 = operator_new(0x8a8);
    uStack_4 = 1;
    if (pvVar2 == (void *)0x0) {
      piVar3 = (int *)0x0;
    }
    else {
      piVar3 = OkayDialog_ctor(pvVar2,*(int *)((int)this + 0x878),"NOT UNLOCKED!",
                               (byte *)
                               "THE MIRROR TOURNAMENT ISN\'T UNLOCKED YET!  TO UNLOCK THE MIRROR TOURNAMENT, YOU NEED TO WIN A TOURNAMENT AT NORMAL OR FRENZIED DIFFICULTY!"
                               ,600);
    }
    uStack_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
  }
  iVar4 = 4;
  bVar7 = true;
  pcVar5 = param_1;
  pcVar6 = "1PT";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *pcVar5 == *pcVar6;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  } while (bVar7);
  if (bVar7) {
    iVar4 = *(int *)((int)this + 0x878);
    *(undefined1 *)(iVar4 + 0x5d5) = 0;
    *(undefined1 *)(iVar4 + 0x5d7) = 0;
    *(undefined1 *)(iVar4 + 0x5d4) = 1;
    *(undefined1 *)(*(int *)((int)this + 0x878) + 0x235) = 1;
    *(undefined1 *)(*(int *)((int)this + 0x878) + 0x236) = 0;
    iVar4 = _check_file_access("DATA\\TOURNAMENT.SAV",0);
    if ((iVar4 == -1) || (cVar1 = Game_SetInProgress(*(int *)((int)this + 0x878)), cVar1 == '\0')) {
      (**(code **)(*(int *)this + 0x40))();
      pvVar2 = operator_new(0xcdc);
      uStack_4 = 2;
      if (pvVar2 == (void *)0x0) goto LAB_00433cbc;
      piVar3 = DifficultyMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
    }
    else {
      pvVar2 = operator_new(0x8bc);
      uStack_4 = 3;
      if (pvVar2 == (void *)0x0) {
LAB_00433cbc:
        piVar3 = (int *)0x0;
      }
      else {
        piVar3 = TourneyContinueDialog_Ctor(pvVar2,*(int *)((int)this + 0x878),this);
      }
    }
    uStack_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
  }
  iVar4 = 5;
  bVar7 = true;
  pcVar5 = param_1;
  pcVar6 = "1PMT";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *pcVar5 == *pcVar6;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  } while (bVar7);
  if (!bVar7) goto LAB_00433db0;
  iVar4 = *(int *)((int)this + 0x878);
  *(undefined1 *)(iVar4 + 0x5d5) = 0;
  *(undefined1 *)(iVar4 + 0x5d7) = 0;
  *(undefined1 *)(iVar4 + 0x5d4) = 1;
  *(undefined1 *)(*(int *)((int)this + 0x878) + 0x235) = 1;
  *(undefined1 *)(*(int *)((int)this + 0x878) + 0x236) = 1;
  iVar4 = _check_file_access("DATA\\TOURNAMENT.SAV",0);
  if (iVar4 == -1) {
    (**(code **)(*(int *)this + 0x40))();
    pvVar2 = operator_new(0xcdc);
    uStack_4 = 4;
    if (pvVar2 == (void *)0x0) {
LAB_00433d93:
      piVar3 = (int *)0x0;
    }
    else {
      piVar3 = DifficultyMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
    }
  }
  else {
    pvVar2 = operator_new(0x8bc);
    uStack_4 = 5;
    if (pvVar2 == (void *)0x0) goto LAB_00433d93;
    piVar3 = TourneyContinueDialog_Ctor(pvVar2,*(int *)((int)this + 0x878),this);
  }
  uStack_4 = 0xffffffff;
  Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
LAB_00433db0:
  iVar4 = 4;
  bVar7 = true;
  pcVar5 = param_1;
  pcVar6 = "1PP";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *pcVar5 == *pcVar6;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  } while (bVar7);
  if (bVar7) {
    *(undefined1 *)(*(int *)((int)this + 0x878) + 0x235) = 0;
    (**(code **)(*(int *)this + 0x40))();
    pvVar2 = operator_new(0xd18);
    uStack_4 = 6;
    if (pvVar2 == (void *)0x0) {
      piVar3 = (int *)0x0;
    }
    else {
      piVar3 = PracticeMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
    }
    uStack_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
  }
  iVar4 = 5;
  bVar7 = true;
  pcVar5 = param_1;
  pcVar6 = "1PTT";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *pcVar5 == *pcVar6;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  } while (bVar7);
  if (bVar7) {
    *(undefined1 *)(*(int *)((int)this + 0x878) + 0x235) = 0;
    (**(code **)(*(int *)this + 0x40))();
    pvVar2 = operator_new(0xd18);
    uStack_4 = 7;
    if (pvVar2 == (void *)0x0) {
      piVar3 = (int *)0x0;
    }
    else {
      piVar3 = TimeTrialMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
    }
    uStack_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
  }
  iVar4 = 6;
  bVar7 = true;
  pcVar5 = "PARTY";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar7 = *param_1 == *pcVar5;
    param_1 = param_1 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar7);
  if (bVar7) {
    (**(code **)(*(int *)this + 0x40))();
    pvVar2 = operator_new(0xcdc);
    uStack_4 = 8;
    if (pvVar2 == (void *)0x0) {
      piVar3 = (int *)0x0;
    }
    else {
      piVar3 = MPMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
    }
    uStack_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
  }
  ExceptionList = local_c;
  return;
}

