
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall UIList_ScrollUpdate(int *param_1)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  float fStack_10;
  float fStack_c;
  
  bVar1 = true;
  if (((char)param_1[0x334] != '\0') && (iVar3 = param_1[0x330], iVar3 != 0)) {
    if (_DAT_004d6ac8 <
        *(float *)(*(int *)(iVar3 + 0x1c) + 0x10) + *(float *)(*(int *)(iVar3 + 0x1c) + 8) +
        (float)param_1[0x333]) {
      param_1[0x335] = 1;
    }
    if (*(float *)(*(int *)(iVar3 + 0x1c) + 8) + (float)param_1[0x333] < _DAT_004cf3ec) {
      param_1[0x336] = 1;
    }
  }
  if (param_1[0x335] != 0) {
    iVar3 = param_1[0x333];
    param_1[0x336] = 0;
    param_1[0x333] = iVar3 + -5;
    if (iVar3 + -5 < param_1[0x332]) {
      param_1[0x333] = param_1[0x332];
      param_1[0x335] = 0;
    }
  }
  if (param_1[0x336] != 0) {
    iVar3 = param_1[0x333];
    param_1[0x335] = 0;
    param_1[0x333] = iVar3 + 5;
    if (param_1[0x331] < iVar3 + 5) {
      param_1[0x333] = param_1[0x331];
      param_1[0x336] = 0;
    }
  }
  if ((char)param_1[0x32f] != '\0') {
    (**(code **)(*param_1 + 0x50))();
  }
  if (*(int **)(*(int *)(param_1[0x21e] + 0x184) + 0x424) == param_1) {
    uVar2 = Input_CheckKeyCombo((void *)param_1[0x21e],2);
    if ((char)uVar2 != '\0') {
      iVar3 = AthenaList_FindByValue(param_1 + 0x223,param_1[0x330]);
      if (iVar3 != -1) {
        (**(code **)(*param_1 + 0x48))(*(undefined4 *)(param_1[0x330] + 4));
      }
    }
    if (param_1[0x32e] != 0) {
      iVar3 = 0x550;
      do {
        if (*(int *)((int)*(void **)(iVar3 + param_1[0x21e]) + 8) != 2) {
          Ball_GetInputForce(*(void **)(iVar3 + param_1[0x21e]),&fStack_10);
          if ((fStack_c != _DAT_004cf368) && (fStack_10 == _DAT_004cf368)) {
            if (_DAT_004cf368 < fStack_c) {
              bVar1 = false;
            }
            if (fStack_c < _DAT_004cf368) {
              bVar1 = false;
            }
          }
        }
        iVar3 = iVar3 + 4;
      } while (iVar3 < 0x560);
      if (bVar1) {
        param_1[0x32e] = 0;
      }
    }
    iVar3 = param_1[0x32e];
    param_1[0x32e] = iVar3 + -1;
    if (iVar3 + -1 < 1) {
      param_1[0x32e] = 0;
      iVar3 = 0x550;
      do {
        if (*(int *)((int)*(void **)(iVar3 + param_1[0x21e]) + 8) != 2) {
          Ball_GetInputForce(*(void **)(iVar3 + param_1[0x21e]),&fStack_10);
          if ((fStack_c != _DAT_004cf368) && (fStack_10 == _DAT_004cf368)) {
            if (_DAT_004cf368 < fStack_c) {
              (**(code **)(*param_1 + 0x20))(0x410);
              param_1[0x32e] = 0xf;
            }
            if (fStack_c < _DAT_004cf368) {
              (**(code **)(*param_1 + 0x20))(0x40e);
              param_1[0x32e] = 0xf;
            }
          }
        }
        iVar3 = iVar3 + 4;
      } while (iVar3 < 0x560);
    }
  }
  return;
}

