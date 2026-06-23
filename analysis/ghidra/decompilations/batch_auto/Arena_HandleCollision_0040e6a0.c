/*
 * Function: Arena_HandleCollision
 * Address: 0x0040e6a0
 * Signature: void __thiscall Arena_HandleCollision(void *this,int *param_1,int *param_2)
 *
 * Patterns: allocates, SEH frame, audio, collision, ball. Calls: Arena_HandleCollision, __stricmp, CreateBonkPopup, Hammer_ChaseStart, Saw_AlertActivate, Saw_Activate, AthenaList_NextIndex, Judge_Reset. Offsets: 18, Lines: 110
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Arena_HandleCollision(void *this,int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  long lVar3;
  int iVar4;
  void *pvVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c99bb;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:CALLHAMMER");
  if ((iVar2 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    CreateBonkPopup(*(int *)((int)this + 0x436c));
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:HAMMERCHASE");
  if ((iVar2 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Hammer_ChaseStart(*(int *)((int)this + 0x436c));
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:ALERTSAW1");
  if ((iVar2 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Saw_AlertActivate(*(int *)((int)this + 0x4370));
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:ALERTSAW2");
  if ((iVar2 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Saw_AlertActivate(*(int *)((int)this + 0x4374));
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:ACTIVATESAW1");
  if ((iVar2 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Saw_Activate(*(int *)((int)this + 0x4370));
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:ACTIVATESAW2");
  if ((iVar2 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    Saw_Activate(*(int *)((int)this + 0x4374));
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:ALERTJUDGES");
  if (iVar2 == 0) {
    iVar2 = AthenaList_NextIndex((int)this + 0x4bbc);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x4bc4) = 0;
    if (*(int *)((int)this + 0x4bc0) < 1) {
      iVar4 = 0;
    }
    else {
      iVar4 = **(int **)((int)this + 0x4fc8);
      *(undefined4 *)((int)this + iVar2 * 4 + 0x4bc4) = 1;
    }
    while (iVar4 != 0) {
      Judge_Reset(iVar4);
      iVar1 = *(int *)((int)this + iVar2 * 4 + 0x4bc4);
      if (*(int *)((int)this + 0x4bc0) <= iVar1) break;
      iVar4 = *(int *)(*(int *)((int)this + 0x4fc8) + iVar1 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x4bc4) = iVar1 + 1;
    }
  }
  iVar2 = __strnicmp(*(char **)(param_2[1] + 0x864),"E:SCORE",7);
  if (iVar2 == 0) {
    iVar2 = *(int *)(param_2[1] + 0x864);
    iVar4 = AthenaList_NextIndex((int)this + 0x4bbc);
    pvVar5 = (void *)0x0;
    *(undefined4 *)((int)this + iVar4 * 4 + 0x4bc4) = 0;
    if (0 < *(int *)((int)this + 0x4bc0)) {
      pvVar5 = (void *)**(undefined4 **)((int)this + 0x4fc8);
      *(undefined4 *)((int)this + iVar4 * 4 + 0x4bc4) = 1;
    }
    while (pvVar5 != (void *)0x0) {
      lVar3 = _atol((char *)(iVar2 + 7));
      ScoreDisplay_SetTime(pvVar5,lVar3);
      iVar1 = *(int *)((int)this + iVar4 * 4 + 0x4bc4);
      if (*(int *)((int)this + 0x4bc0) <= iVar1) break;
      pvVar5 = *(void **)(*(int *)((int)this + 0x4fc8) + iVar1 * 4);
      *(int *)((int)this + iVar4 * 4 + 0x4bc4) = iVar1 + 1;
    }
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:JUMP");
  if ((iVar2 == 0) && (param_1[0x1f7] < 1)) {
    Sound_Play3D(*(void **)(*(int *)((int)this + 0x878) + 0x49c),(float)param_1[0x59],
                 (float)param_1[0x5a],(float)param_1[0x5b]);
    param_1[0x1f7] = 10;
    param_1[0xa7] = 0x3b03126f;
    param_1[0xa8] = 1;
    param_1[0x202] = 10;
    Ball_RecordBest(param_1,200);
  }
  iVar2 = __strnicmp(*(char **)(param_2[1] + 0x864),"E:BELL",6);
  if (iVar2 == 0) {
    Bell_Activate(*(int *)((int)this + 0x4fd4));
    iVar2 = *(int *)(*(int *)((int)this + 0x878) + 0x220);
    if ((*(char *)(iVar2 + 0x10) == '\0') && (*(char *)(iVar2 + 0x11) == '\0')) {
      *(undefined4 *)(param_1[6] * 0xa0 + 0x5ec + *(int *)((int)this + 0x878)) = 500;
      pvVar5 = operator_new(0x30);
      iVar2 = 0;
      local_4 = 0;
      if (pvVar5 != (void *)0x0) {
        iVar2 = ScoreObject_ctor(pvVar5,(int)this,
                                 param_1[6] * 0xa0 + 0x5cc + *(int *)((int)this + 0x878),
                                 "EXTRA TIME:");
      }
      local_4 = 0xffffffff;
      Timer_Decrement(iVar2);
      AthenaList_Append((void *)((int)this + 0x8b8),iVar2);
    }
  }
  DispatchCollisionEvents(this,param_1,param_2);
  ExceptionList = local_c;
  return;
}
