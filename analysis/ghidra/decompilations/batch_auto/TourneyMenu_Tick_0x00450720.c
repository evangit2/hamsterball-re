
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall TourneyMenu_Tick(int *param_1)

{
  float fVar1;
  bool bVar2;
  float fVar3;
  int iVar4;
  int iVar5;
  
  RumbleBoard_TickTimer((int)(param_1 + 0x337));
  if ((char)param_1[0x442] != '\0') {
    iVar4 = param_1[0x443];
    if (iVar4 == 0) {
      if ((float)param_1[0x444] == _DAT_004cf368) {
        param_1[0x444] = 0x3d4ccccd;
      }
      fVar1 = (float)param_1[0x444];
      fVar3 = (float)_DAT_004d5d80;
      param_1[0x444] = (int)(fVar1 * fVar3);
      param_1[0x445] = (int)((float)param_1[0x445] + _DAT_004d85c8);
      if (_DAT_004cf310 <= fVar1 * fVar3) {
        param_1[0x444] = 0x3f800000;
        param_1[0x446] = 0xfa;
        param_1[0x443] = 1;
      }
    }
    else if (iVar4 == 1) {
      iVar4 = param_1[0x446];
      param_1[0x446] = iVar4 + -1;
      if (iVar4 + -1 < 1) {
        param_1[0x443] = 2;
      }
    }
    else if (iVar4 == 2) {
      fVar1 = (float)param_1[0x444];
      fVar3 = (float)_DAT_004cf508;
      param_1[0x444] = (int)(fVar1 * fVar3);
      param_1[0x445] = (int)((float)param_1[0x445] - _DAT_004cf9f8);
      if (fVar1 * fVar3 < (float)_DAT_004cf428) {
        param_1[0x444] = 0;
        *(undefined1 *)(param_1 + 0x442) = 0;
        param_1[0x443] = 3;
      }
    }
  }
  fVar1 = (float)param_1[0x33e] + (float)_DAT_004d03e0;
  bVar2 = _DAT_004cf418 <= fVar1;
  param_1[0x33e] = (int)fVar1;
  if (bVar2) {
    param_1[0x33e] = (int)(fVar1 - _DAT_004cf418);
  }
  UIList_ScrollUpdate(param_1);
  if (((char)param_1[0x33d] != '\0') &&
     (fVar1 = (float)param_1[0x33c] + _DAT_004cf448, param_1[0x33c] = (int)fVar1,
     _DAT_004cf310 <= fVar1)) {
    (**(code **)(*param_1 + 0x40))();
    Tournament_AdvanceRace(*(void **)(param_1[0x21e] + 0x220),'\0');
  }
  iVar4 = (*(int *)(*(int *)(param_1[0x21e] + 0x220) + 8) * 0x9b - param_1[0x33f]) + 0x16;
  iVar5 = 3;
  do {
    if (0x145 < iVar4) {
      param_1[0x33f] = param_1[0x33f] + 1;
    }
    if (iVar4 < 0x16) {
      param_1[0x33f] = param_1[0x33f] + -1;
    }
    iVar5 = iVar5 + -1;
  } while (iVar5 != 0);
  return;
}

