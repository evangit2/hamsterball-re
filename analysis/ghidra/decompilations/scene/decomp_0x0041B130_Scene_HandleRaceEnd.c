// Decompiled via GhidraMCP
// Function: Scene_HandleRaceEnd
// Address: 0x0041B130
// Category: scene

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_HandleRaceEnd(int param_1)

{
  int iVar1;
  float fVar2;
  char cVar3;
  int iVar4;
  void *pvVar5;
  int *piVar6;
  int iVar7;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c9736;
  pvStack_c = ExceptionList;
  piVar6 = (int *)0x0;
  ExceptionList = &pvStack_c;
  if (*(int *)(param_1 + 0x3f18) != 0) {
    ExceptionList = &pvStack_c;
    FlagWaver_AdvancePhase(*(int *)(param_1 + 0x3f18));
  }
  iVar4 = AthenaList_NextIndex(param_1 + 0x2578);
  *(undefined4 *)(param_1 + 0x2580 + iVar4 * 4) = 0;
  if (0 < *(int *)(param_1 + 0x257c)) {
    piVar6 = (int *)**(undefined4 **)(param_1 + 0x2984);
    *(undefined4 *)(param_1 + 0x2580 + iVar4 * 4) = 1;
  }
  while (piVar6 != (int *)0x0) {
    cVar3 = (**(code **)(*piVar6 + 0x2c))();
    if (cVar3 == '\0') {
      thunk_Gfx_SetRenderState((void *)(param_1 + 0x2578),(int)piVar6);
    }
    iVar7 = *(int *)(param_1 + 0x2580 + iVar4 * 4);
    if (*(int *)(param_1 + 0x257c) <= iVar7) break;
    piVar6 = *(int **)(*(int *)(param_1 + 0x2984) + iVar7 * 4);
    *(int *)(param_1 + 0x2580 + iVar4 * 4) = iVar7 + 1;
  }
  fVar2 = *(float *)(param_1 + 0x3624) - (float)_DAT_004d03e0;
  *(float *)(param_1 + 0x3624) = fVar2;
  if (fVar2 < _DAT_004cf368) {
    *(undefined4 *)(param_1 + 0x3624) = 0;
  }
  if (*(int *)(param_1 + 0x3620) == 0x19) {
    iVar4 = AthenaList_NextIndex(param_1 + 0x362c);
    *(undefined4 *)(param_1 + 0x3634 + iVar4 * 4) = 0;
    if (*(int *)(param_1 + 0x3630) < 1) {
      iVar7 = 0;
    }
    else {
      iVar7 = **(int **)(param_1 + 0x3a38);
      *(undefined4 *)(param_1 + 0x3634 + iVar4 * 4) = 1;
    }
    while (iVar7 != 0) {
      if (*(int *)(iVar7 + 0x20) != 0) {
        pvVar5 = operator_new(0x30);
        uStack_4 = 0;
        if (pvVar5 == (void *)0x0) {
          iVar7 = 0;
        }
        else {
          iVar7 = ScoreObject_ctor(pvVar5,param_1,iVar7,"RACE TIME:");
        }
        uStack_4 = 0xffffffff;
        AthenaList_Append((void *)(param_1 + 0x8b8),iVar7);
      }
      iVar1 = *(int *)(param_1 + 0x3634 + iVar4 * 4);
      if (*(int *)(param_1 + 0x3630) <= iVar1) break;
      iVar7 = *(int *)(*(int *)(param_1 + 0x3a38) + iVar1 * 4);
      *(int *)(param_1 + 0x3634 + iVar4 * 4) = iVar1 + 1;
    }
  }
  if (*(char *)(param_1 + 0x3a4c) == '\0') {
    if ((*(int *)(param_1 + 0x8b4) == 0) && (0x19 < *(int *)(param_1 + 0x3620))) {
      iVar4 = 0;
      do {
        fVar2 = *(float *)(param_1 + 0x3a54) + (float)_DAT_004cf538;
        *(float *)(param_1 + 0x3a54) = fVar2;
        if (((*(int *)(param_1 + 0x3a50) == 2) && ((float)_DAT_004cf4f8 <= fVar2)) &&
           (*(char *)(param_1 + 0x3a58) != '\0')) {
          Sound_PlayChannel(*(int *)(*(int *)(param_1 + 0x878) + 0x444));
          *(undefined1 *)(param_1 + 0x3a58) = 0;
        }
        if ((float)((uint)(*(int *)(param_1 + 0x3a50) == 2) << 1) + (float)_DAT_004d03c8 <
            *(float *)(param_1 + 0x3a54)) {
          iVar7 = *(int *)(param_1 + 0x3a50) + 1;
          *(undefined4 *)(param_1 + 0x3a54) = 0;
          *(int *)(param_1 + 0x3a50) = iVar7;
          if (2 < iVar7) {
            *(undefined1 *)(param_1 + 0x3a4c) = 1;
            break;
          }
        }
        iVar4 = iVar4 + 1;
      } while (iVar4 < 3);
    }
    if (*(char *)(param_1 + 0x3a4c) == '\0') {
      ExceptionList = pvStack_c;
      return;
    }
  }
  iVar4 = *(int *)(*(int *)(param_1 + 0x878) + 0x220);
  if ((*(char *)(iVar4 + 0x10) == '\0') && (*(char *)(iVar4 + 0x11) == '\0')) {
    iVar4 = AthenaList_NextIndex(param_1 + 0x362c);
    *(undefined4 *)(param_1 + 0x3634 + iVar4 * 4) = 0;
    if (*(int *)(param_1 + 0x3630) < 1) {
      iVar7 = 0;
    }
    else {
      iVar7 = **(int **)(param_1 + 0x3a38);
      *(undefined4 *)(param_1 + 0x3634 + iVar4 * 4) = 1;
    }
    if (iVar7 != 0) {
      while( true ) {
        if (*(char *)(iVar7 + 10) == '\0') {
          piVar6 = (int *)(iVar7 + 0x1c);
          *piVar6 = *piVar6 + -1;
          if ((*piVar6 < 0) && (*(int *)(iVar7 + 0x20) < 1)) {
            *(undefined4 *)(iVar7 + 0x1c) = 0;
            *(undefined1 *)(iVar7 + 10) = 1;
            *(undefined1 *)(*(int *)(iVar7 + 0x10) + 0x14c) = 1;
            Scene_UpdateChildren(*(int *)(*(int *)(param_1 + 0x878) + 0x178));
            *(undefined1 *)(param_1 + 0x880) = 1;
            NoOp_return();
            pvVar5 = operator_new(0x60);
            uStack_4 = 1;
            if (pvVar5 == (void *)0x0) {
              pvVar5 = (void *)0x0;
            }
            else {
              pvVar5 = RaceResultPopup_ctor(pvVar5,param_1,iVar7);
            }
            uStack_4 = 0xffffffff;
            AthenaList_Append((void *)(param_1 + 0x8b8),(int)pvVar5);
            Audio_StopChannel(*(int *)(*(int *)(param_1 + 0x878) + 0x53c));
            Audio_PlayMusic(*(void **)(*(int *)(param_1 + 0x878) + 0x53c),"Game Over");
          }
        }
        iVar1 = *(int *)(param_1 + 0x3634 + iVar4 * 4);
        if (*(int *)(param_1 + 0x3630) <= iVar1) break;
        iVar7 = *(int *)(*(int *)(param_1 + 0x3a38) + iVar1 * 4);
        *(int *)(param_1 + 0x3634 + iVar4 * 4) = iVar1 + 1;
        if (iVar7 == 0) {
          ExceptionList = pvStack_c;
          return;
        }
      }
    }
  }
  else {
    iVar4 = AthenaList_NextIndex(param_1 + 0x362c);
    *(undefined4 *)(param_1 + 0x3634 + iVar4 * 4) = 0;
    if (*(int *)(param_1 + 0x3630) < 1) {
      iVar7 = 0;
    }
    else {
      iVar7 = **(int **)(param_1 + 0x3a38);
      *(undefined4 *)(param_1 + 0x3634 + iVar4 * 4) = 1;
    }
    while (iVar7 != 0) {
      if (*(char *)(iVar7 + 10) == '\0') {
        *(int *)(iVar7 + 0x1c) = *(int *)(iVar7 + 0x1c) + 1;
      }
      iVar1 = *(int *)(param_1 + 0x3634 + iVar4 * 4);
      if (*(int *)(param_1 + 0x3630) <= iVar1) {
        ExceptionList = pvStack_c;
        return;
      }
      iVar7 = *(int *)(*(int *)(param_1 + 0x3a38) + iVar1 * 4);
      *(int *)(param_1 + 0x3634 + iVar4 * 4) = iVar1 + 1;
    }
  }
  ExceptionList = pvStack_c;
  return;
}