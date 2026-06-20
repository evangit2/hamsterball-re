
void __thiscall OptionsMenu_HandleButtonClick(void *this,char *param_1)

{
  void *pvVar1;
  int *piVar2;
  int iVar3;
  char *pcVar4;
  char *pcVar5;
  bool bVar6;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  pvVar1 = ExceptionList;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb473;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined1 *)(*(int *)((int)this + 0x878) + 0x234) = 1;
  if (param_1 == (char *)0x0) {
    ExceptionList = pvVar1;
    return;
  }
  iVar3 = 9;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "CONTROL1";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    piVar2 = (int *)(*(int *)((int)this + 0x878) + 0xb28);
    *piVar2 = *piVar2 + 1;
    if (3 < *(int *)(*(int *)((int)this + 0x878) + 0xb28)) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb28) = 0;
    }
    iVar3 = __stricmp((char *)**(undefined4 **)
                                (*(int *)((int)this + 0x878) + 0x550 +
                                *(int *)(*(int *)((int)this + 0x878) + 0xb28) * 4),"NOT CONNECTED");
    if (iVar3 == 0) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb28) = 0;
    }
    OptionsMenu_UpdateControl1(this);
  }
  iVar3 = 9;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "CONTROL2";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    piVar2 = (int *)(*(int *)((int)this + 0x878) + 0xb2c);
    *piVar2 = *piVar2 + 1;
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb2c) == 4) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb2c) = 99;
    }
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb2c) == 0x65) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb2c) = 0;
    }
    iVar3 = *(int *)(*(int *)((int)this + 0x878) + 0xb2c);
    if ((iVar3 < 99) &&
       (iVar3 = __stricmp((char *)**(undefined4 **)(*(int *)((int)this + 0x878) + 0x550 + iVar3 * 4)
                          ,"NOT CONNECTED"), iVar3 == 0)) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb2c) = 99;
    }
    OptionsMenu_UpdateControl2(this);
  }
  iVar3 = 9;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "CONTROL3";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    piVar2 = (int *)(*(int *)((int)this + 0x878) + 0xb30);
    *piVar2 = *piVar2 + 1;
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb30) == 4) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb30) = 99;
    }
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb30) == 0x65) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb30) = 0;
    }
    iVar3 = *(int *)(*(int *)((int)this + 0x878) + 0xb30);
    if ((iVar3 < 99) &&
       (iVar3 = __stricmp((char *)**(undefined4 **)(*(int *)((int)this + 0x878) + 0x550 + iVar3 * 4)
                          ,"NOT CONNECTED"), iVar3 == 0)) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb30) = 99;
    }
    OptionsMenu_UpdateControl3(this);
  }
  iVar3 = 9;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "CONTROL4";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    piVar2 = (int *)(*(int *)((int)this + 0x878) + 0xb34);
    *piVar2 = *piVar2 + 1;
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb34) == 4) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb34) = 99;
    }
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb34) == 0x65) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb34) = 0;
    }
    iVar3 = *(int *)(*(int *)((int)this + 0x878) + 0xb34);
    if ((iVar3 < 99) &&
       (iVar3 = __stricmp((char *)**(undefined4 **)(*(int *)((int)this + 0x878) + 0x550 + iVar3 * 4)
                          ,"NOT CONNECTED"), iVar3 == 0)) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb34) = 99;
    }
    OptionsMenu_UpdateControl4(this);
  }
  iVar3 = 7;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "2PRACE";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    if (*(int *)(*(int *)((int)this + 0x878) + 0xb2c) < 99) {
      (**(code **)(*(int *)this + 0x40))();
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb38) = 0;
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb3c) = 0;
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb40) = 0;
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb44) = 0;
      pvVar1 = operator_new(0xd18);
      local_4 = 1;
      if (pvVar1 == (void *)0x0) {
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = PartyMenu_ctor(pvVar1,*(int *)((int)this + 0x878));
      }
      pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x184);
    }
    else {
      pvVar1 = operator_new(0x8a8);
      local_4 = 0;
      if (pvVar1 == (void *)0x0) {
        piVar2 = (int *)0x0;
        pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x184);
      }
      else {
        piVar2 = OkayDialog_ctor(pvVar1,*(int *)((int)this + 0x878),"TWO PLAYERS!",
                                 (byte *)
                                 "THE PARTY RACE REQUIRES THAT PLAYER 1 AND PLAYER 2 BE HUMAN PLAYERS!"
                                 ,600);
        pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x184);
      }
    }
    local_4 = 0xffffffff;
    Scene_AddObject(pvVar1,piVar2);
  }
  iVar3 = 7;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "RUMBLE";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    iVar3 = *(int *)((int)this + 0x878);
    if (((*(int *)(iVar3 + 0xb2c) == 100) && (*(int *)(iVar3 + 0xb30) == 100)) &&
       (*(int *)(iVar3 + 0xb34) == 100)) {
      pvVar1 = operator_new(0x8a8);
      local_4 = 2;
      if (pvVar1 == (void *)0x0) {
        pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x184);
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = OkayDialog_ctor(pvVar1,*(int *)((int)this + 0x878),"TWO PLAYERS!",
                                 (byte *)
                                 "THE RODENT RUMBLE REQUIRES AT LEAST TWO HUMAN OR COMPUTER PLAYERS!"
                                 ,400);
        pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x184);
      }
    }
    else {
      (**(code **)(*(int *)this + 0x40))();
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb38) = 0;
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb3c) = 0;
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb40) = 0;
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0xb44) = 0;
      pvVar1 = operator_new(0xcdc);
      local_4 = 3;
      if (pvVar1 == (void *)0x0) {
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = ArenaMenu_ctor(pvVar1,*(int *)((int)this + 0x878));
      }
      pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x184);
    }
    local_4 = 0xffffffff;
    Scene_AddObject(pvVar1,piVar2);
  }
  iVar3 = 5;
  bVar6 = true;
  pcVar4 = param_1;
  pcVar5 = "BACK";
  do {
    if (iVar3 == 0) break;
    iVar3 = iVar3 + -1;
    bVar6 = *pcVar4 == *pcVar5;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  } while (bVar6);
  if (bVar6) {
    (**(code **)(*(int *)this + 0x40))();
    pvVar1 = operator_new(0xcdc);
    local_4 = 4;
    if (pvVar1 == (void *)0x0) {
      piVar2 = (int *)0x0;
    }
    else {
      piVar2 = TournamentScreen_ctor(pvVar1,*(int *)((int)this + 0x878));
    }
    local_4 = 0xffffffff;
    Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar2);
  }
  iVar3 = __strnicmp(param_1,"LOCKED",6);
  if (iVar3 == 0) {
    pvVar1 = operator_new(0x8a8);
    local_4 = 5;
    if (pvVar1 != (void *)0x0) {
      piVar2 = OkayDialog_ctor(pvVar1,*(int *)((int)this + 0x878),"NOT UNLOCKED!",
                               (byte *)
                               "THE MIRROR TOURNAMENT ISN\'T UNLOCKED YET!  TO UNLOCK THE MIRROR TOURNAMENT, YOU NEED TO WIN A TOURNAMENT AT NORMAL OR FRENZIED DIFFICULTY!"
                               ,600);
      goto LAB_00431487;
    }
  }
  else {
    iVar3 = 4;
    bVar6 = true;
    pcVar4 = param_1;
    pcVar5 = "2PT";
    do {
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      bVar6 = *pcVar4 == *pcVar5;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    } while (bVar6);
    if (bVar6) {
      (**(code **)(*(int *)this + 0x40))();
      iVar3 = *(int *)((int)this + 0x878);
      *(undefined1 *)(iVar3 + 0x5d5) = 0;
      *(undefined1 *)(iVar3 + 0x5d7) = 0;
      *(undefined1 *)(iVar3 + 0x5d4) = 1;
      iVar3 = *(int *)((int)this + 0x878);
      *(undefined1 *)(iVar3 + 0x675) = 0;
      *(undefined1 *)(iVar3 + 0x677) = 0;
      *(undefined1 *)(iVar3 + 0x674) = 1;
      *(undefined1 *)(*(int *)((int)this + 0x878) + 0x235) = 1;
      *(undefined1 *)(*(int *)((int)this + 0x878) + 0x236) = 0;
      pvVar1 = operator_new(0xcdc);
      local_4 = 6;
      if (pvVar1 == (void *)0x0) {
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = DifficultyMenu_ctor(pvVar1,*(int *)((int)this + 0x878));
      }
      local_4 = 0xffffffff;
      Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar2);
    }
    iVar3 = 5;
    bVar6 = true;
    pcVar4 = param_1;
    pcVar5 = "2PMT";
    do {
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      bVar6 = *pcVar4 == *pcVar5;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    } while (bVar6);
    if (bVar6) {
      (**(code **)(*(int *)this + 0x40))();
      iVar3 = *(int *)((int)this + 0x878);
      *(undefined1 *)(iVar3 + 0x5d5) = 0;
      *(undefined1 *)(iVar3 + 0x5d7) = 0;
      *(undefined1 *)(iVar3 + 0x5d4) = 1;
      iVar3 = *(int *)((int)this + 0x878);
      *(undefined1 *)(iVar3 + 0x675) = 0;
      *(undefined1 *)(iVar3 + 0x677) = 0;
      *(undefined1 *)(iVar3 + 0x674) = 1;
      *(undefined1 *)(*(int *)((int)this + 0x878) + 0x235) = 1;
      *(undefined1 *)(*(int *)((int)this + 0x878) + 0x236) = 1;
      pvVar1 = operator_new(0xcdc);
      local_4 = 7;
      if (pvVar1 == (void *)0x0) {
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = DifficultyMenu_ctor(pvVar1,*(int *)((int)this + 0x878));
      }
      local_4 = 0xffffffff;
      Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar2);
    }
    iVar3 = 4;
    bVar6 = true;
    pcVar4 = "2PP";
    do {
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      bVar6 = *param_1 == *pcVar4;
      param_1 = param_1 + 1;
      pcVar4 = pcVar4 + 1;
    } while (bVar6);
    if (!bVar6) {
      ExceptionList = local_c;
      return;
    }
    (**(code **)(*(int *)this + 0x40))();
    pvVar1 = operator_new(0xd18);
    local_4 = 8;
    if (pvVar1 != (void *)0x0) {
      piVar2 = PracticeMenu_ctor(pvVar1,*(int *)((int)this + 0x878));
      goto LAB_00431487;
    }
  }
  piVar2 = (int *)0x0;
LAB_00431487:
  local_4 = 0xffffffff;
  Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar2);
  ExceptionList = local_c;
  return;
}

