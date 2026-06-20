/*
 * Function: RumbleBoard_Update
 * Address: 0x00421fe0
 * Signature: void __fastcall RumbleBoard_Update(int *param_1)
 *
 * Patterns: allocates, SEH frame, audio, scene, board, menu. Calls: RumbleBoard_Update, Scene_Update, RumbleBoard_TickTimer, AthenaList_NextIndex, operator_new, RaceResultsMenu_ctor, AthenaList_Append, Scene_UpdateChildren. Offsets: 15, Lines: 133
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall RumbleBoard_Update(int *param_1)

{
  int iVar1;
  int iVar2;
  void *this;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  bool bVar8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c965b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Scene_Update(param_1);
  RumbleBoard_TickTimer((int)(param_1 + 0x11f2));
  if ((((char)param_1[0xe93] != '\0') &&
      (bVar8 = (char)param_1[0x11ec] == '\0', *(bool *)(param_1 + 0x11ec) = bVar8, bVar8)) &&
     (iVar7 = param_1[0x11eb], param_1[0x11eb] = iVar7 + -1, iVar7 + -1 < 1)) {
    if ((char)param_1[0x11f1] == '\0') {
      iVar7 = param_1[0x11f0];
      iVar2 = param_1[0x11ef];
      iVar5 = iVar2;
      if (iVar2 <= iVar7) {
        iVar5 = iVar7;
      }
      iVar1 = param_1[0x11ee];
      iVar3 = iVar1;
      if ((iVar1 <= iVar5) && (iVar3 = iVar2, iVar2 <= iVar7)) {
        iVar3 = iVar7;
      }
      iVar5 = param_1[0x11ed];
      if (iVar5 <= iVar3) {
        iVar3 = iVar2;
        if (iVar2 <= iVar7) {
          iVar3 = iVar7;
        }
        iVar5 = iVar1;
        if ((iVar1 <= iVar3) && (iVar5 = iVar2, iVar2 <= iVar7)) {
          iVar5 = iVar7;
        }
      }
      iVar3 = param_1[0x21e];
      uVar6 = 0;
      if ((*(char *)(iVar3 + 0x5d7) == '\0') && (param_1[0x11ed] == iVar5)) {
        uVar6 = 1;
      }
      if ((*(char *)(iVar3 + 0x677) == '\0') && (param_1[0x11ee] == iVar5)) {
        uVar6 = uVar6 + 1;
      }
      if ((*(char *)(iVar3 + 0x717) == '\0') && (param_1[0x11ef] == iVar5)) {
        uVar6 = uVar6 + 1;
      }
      if ((*(char *)(iVar3 + 0x7b7) == '\0') && (param_1[0x11f0] == iVar5)) {
        uVar6 = uVar6 + 1;
      }
      if (uVar6 < 2) {
        *(undefined1 *)((int)param_1 + 0x47c5) = 0;
        iVar5 = iVar2;
        if (iVar2 <= iVar7) {
          iVar5 = iVar7;
        }
        iVar4 = iVar1;
        if ((iVar1 <= iVar5) && (iVar4 = iVar2, iVar2 <= iVar7)) {
          iVar4 = iVar7;
        }
        iVar5 = param_1[0x11ed];
        if (iVar5 <= iVar4) {
          iVar4 = iVar2;
          if (iVar2 <= iVar7) {
            iVar4 = iVar7;
          }
          iVar5 = iVar1;
          if ((iVar1 <= iVar4) && (iVar5 = iVar2, iVar2 <= iVar7)) {
            iVar5 = iVar7;
          }
        }
        iVar7 = 0;
        if ((*(char *)(iVar3 + 0x677) == '\0') && (param_1[0x11ee] == iVar5)) {
          iVar7 = 1;
        }
        if ((*(char *)(iVar3 + 0x717) == '\0') && (param_1[0x11ef] == iVar5)) {
          iVar7 = 2;
        }
        if ((*(char *)(iVar3 + 0x7b7) == '\0') && (param_1[0x11f0] == iVar5)) {
          iVar7 = 3;
        }
        iVar2 = AthenaList_NextIndex((int)(param_1 + 0xa75));
        param_1[iVar2 + 0xa77] = 0;
        if (param_1[0xa76] < 1) {
          iVar5 = 0;
        }
        else {
          iVar5 = *(int *)param_1[0xb78];
          param_1[iVar2 + 0xa77] = 1;
        }
        while (iVar5 != 0) {
          *(undefined1 *)(iVar5 + 0x14c) = 1;
          iVar1 = param_1[iVar2 + 0xa77];
          if (param_1[0xa76] <= iVar1) break;
          iVar5 = *(int *)(param_1[0xb78] + iVar1 * 4);
          param_1[iVar2 + 0xa77] = iVar1 + 1;
        }
        this = operator_new(0x28);
        local_4 = 0;
        if (this == (void *)0x0) {
          iVar7 = 0;
        }
        else {
          iVar7 = RaceResultsMenu_ctor(this,(int)param_1,iVar7 * 0xa0 + 0x5cc + param_1[0x21e]);
        }
        local_4 = 0xffffffff;
        AthenaList_Append(param_1 + 0x22e,iVar7);
        Scene_UpdateChildren(*(int *)(param_1[0x21e] + 0x178));
        NoOp_return();
        Audio_StopChannel(*(int *)(param_1[0x21e] + 0x53c));
        Audio_PlayMusic(*(void **)(param_1[0x21e] + 0x53c),"Game Over");
        *(undefined1 *)(param_1 + 0x11f1) = 1;
      }
      else {
        *(undefined1 *)((int)param_1 + 0x47c5) = 1;
      }
    }
    param_1[0x11eb] = 0;
  }
  ExceptionList = local_c;
  return;
}
