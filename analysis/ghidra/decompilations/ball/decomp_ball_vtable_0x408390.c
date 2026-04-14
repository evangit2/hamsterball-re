
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00408390(int *param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  uint uVar4;
  char *pcVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  void *this;
  float10 fVar11;
  undefined1 auVar12 [10];
  ulonglong uVar13;
  float fStack_30;
  float fStack_24;
  float fStack_20;
  float fStack_1c;
  float fStack_18;
  float fStack_14;
  float fStack_10;
  float fStack_4;
  
  this = (void *)0x0;
  if ((param_1[0x31d] != 0) || (*(char *)(param_1[4] + 0x237) != '\0')) {
    iVar9 = param_1[0x1da];
    FUN_00405e00(param_1);
    if (((char)param_1[0x1da] == '\0') && ((char)iVar9 != '\0')) {
      iVar3 = AthenaList_GetIndex(param_1[5] + 0x29d4);
      iVar9 = param_1[5];
      *(undefined4 *)(iVar9 + 0x29dc + iVar3 * 4) = 0;
      if (0 < *(int *)(iVar9 + 0x29d8)) {
        this = (void *)**(undefined4 **)(iVar9 + 0x2de0);
        *(undefined4 *)(iVar9 + 0x29dc + iVar3 * 4) = 1;
      }
      while (this != (void *)0x0) {
        if (*(int *)((int)this + 0x18) != -1) {
          uVar13 = FUN_004ba754();
          iVar9 = (int)uVar13;
          uVar13 = FUN_004ba754();
          iVar10 = (int)uVar13;
          uVar13 = FUN_004ba754();
          iVar6 = (int)uVar13;
          uVar13 = FUN_004ba754();
          uVar4 = FUN_00458130((int)uVar13,iVar6,iVar10,iVar9);
          fVar1 = (float)(int)uVar4;
          if ((int)uVar4 < 0) {
            fVar1 = fVar1 + _DAT_004cf558;
          }
          if (fVar1 < _DAT_004cf554) {
            FUN_00428ed0(param_1[4],2000.0);
            uVar13 = FUN_004ba754();
            pcVar5 = (char *)FUN_00466c70(0x4f7448,&DAT_004cf500);
            FUN_00401660(this,pcVar5);
            iVar9 = *(int *)((int)this + 0x18) * 0xa0;
            *(float *)(iVar9 + 0x5e4 + param_1[4]) =
                 (float)(int)uVar13 + *(float *)(iVar9 + 0x5e4 + param_1[4]);
          }
        }
        iVar9 = param_1[5];
        iVar10 = *(int *)(iVar9 + 0x29dc + iVar3 * 4);
        if (*(int *)(iVar9 + 0x29d8) <= iVar10) break;
        this = *(void **)(*(int *)(iVar9 + 0x2de0) + iVar10 * 4);
        *(int *)(iVar9 + 0x29dc + iVar3 * 4) = iVar10 + 1;
      }
    }
    uVar13 = FUN_004ba754();
    iVar9 = (int)uVar13;
    uVar13 = FUN_004ba754();
    iVar3 = (int)uVar13;
    uVar13 = FUN_004ba754();
    iVar10 = (int)uVar13;
    uVar13 = FUN_004ba754();
    uVar4 = FUN_00458130((int)uVar13,iVar10,iVar3,iVar9);
    fVar1 = (float)(int)uVar4;
    if ((int)uVar4 < 0) {
      fVar1 = fVar1 + _DAT_004cf558;
    }
    fStack_30 = 999999.0;
    iVar3 = 0;
    iVar10 = 0;
    iVar6 = AthenaList_GetIndex(param_1[5] + 0x29d4);
    iVar9 = param_1[5];
    *(undefined4 *)(iVar9 + 0x29dc + iVar6 * 4) = 0;
    if (0 < *(int *)(iVar9 + 0x29d8)) {
      iVar3 = **(int **)(iVar9 + 0x2de0);
      *(undefined4 *)(iVar9 + 0x29dc + iVar6 * 4) = 1;
    }
    if (iVar3 != 0) {
      iVar10 = 0;
      do {
        if (((((*(char *)(iVar3 + 0x768) != '\0') && (*(char *)(iVar3 + 0x2f9) == '\0')) &&
             (*(int *)(iVar3 + 0x300) == 0)) &&
            ((*(char *)(iVar3 + 0x324) == '\0' && (*(char *)(param_1[5] + 0x3a4c) != '\0')))) &&
           (*(int *)(iVar3 + 0x18) != -1)) {
          uVar13 = FUN_004ba754();
          iVar9 = (int)uVar13;
          uVar13 = FUN_004ba754();
          iVar7 = (int)uVar13;
          uVar13 = FUN_004ba754();
          iVar8 = (int)uVar13;
          uVar13 = FUN_004ba754();
          uVar4 = FUN_00458130((int)uVar13,iVar8,iVar7,iVar9);
          fVar2 = (float)(int)uVar4;
          if ((int)uVar4 < 0) {
            fVar2 = fVar2 + _DAT_004cf558;
          }
          if (fVar2 < fStack_30) {
            iVar10 = iVar3;
            fStack_30 = fVar2;
          }
        }
        iVar9 = param_1[5];
        iVar7 = *(int *)(iVar9 + 0x29dc + iVar6 * 4);
        if (*(int *)(iVar9 + 0x29d8) <= iVar7) break;
        iVar3 = *(int *)(*(int *)(iVar9 + 0x2de0) + iVar7 * 4);
        *(int *)(iVar9 + 0x29dc + iVar6 * 4) = iVar7 + 1;
      } while (iVar3 != 0);
    }
    if (&fStack_24 != (float *)(param_1 + 0x318)) {
      fStack_24 = (float)param_1[0x318];
      fStack_1c = (float)param_1[0x31a];
      fStack_20 = (float)param_1[0x319];
    }
    fVar11 = (float10)fStack_1c;
    if (fVar1 < _DAT_004cf550 != (fVar1 == _DAT_004cf550)) {
      auVar12 = FUN_00457da0(0x4f7188,(float)param_1[0x31e]);
      fStack_24 = (float)((float10)auVar12 * (float10)(float)param_1[799] + (float10)fStack_24);
      auVar12 = FUN_00457dc0(0x4f7188,(float)param_1[0x31e]);
      fVar11 = (float10)auVar12 * (float10)(float)param_1[799] + (float10)fStack_1c;
    }
    param_1[0x31e] = (int)((float)param_1[0x31e] + _DAT_004cf48c);
    if (((iVar10 != 0) && (fVar1 < (float)param_1[0x31c])) && (fStack_30 < (float)param_1[0x31b])) {
      fStack_24 = *(float *)(iVar10 + 0x164);
      fStack_14 = *(float *)(iVar10 + 0x168);
      fVar11 = (float10)*(float *)(iVar10 + 0x16c);
      fStack_20 = fStack_14;
      if ((float)param_1[0xa1] < *(float *)(iVar10 + 0x284) * (float)_DAT_004cf508) {
        fStack_24 = -fStack_24;
        fStack_20 = -fStack_14;
        fVar11 = (float10)-*(float *)(iVar10 + 0x16c);
      }
    }
    fStack_4 = (float)param_1[0x5b];
    fStack_18 = fStack_24 - (float)param_1[0x59];
    fVar1 = (float)(fVar11 - (float10)fStack_4);
    fVar2 = fVar1 * fVar1 +
            fStack_18 * fStack_18 +
            (fStack_20 - (float)param_1[0x5a]) * (fStack_20 - (float)param_1[0x5a]);
    fStack_10 = _DAT_004cf368;
    if ((fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) &&
       (fStack_10 = SQRT(fVar2), _DAT_004cf368 < fStack_10)) {
      fStack_10 = _DAT_004cf310 / fStack_10;
    }
    fStack_18 = fStack_18 * fStack_10;
    fStack_10 = fStack_10 * fVar1;
    (**(code **)(*param_1 + 0x14))(fStack_18,0,fStack_10);
  }
  return;
}
