/*
 * Function: Level_HandleCollision
 * Address: 0x0040dcd0
 * Signature: Level_HandleCollision(...)
 *
 * Patterns: vtable dispatch, audio, collision, level. Calls: Level_HandleCollision, __stricmp, AthenaList_NextIndex, Catapult_Launch, Sound_PlayChannel, Trapdoor_Open, Trapdoor_Activate, CreateNoDizzy. Offsets: 26, Lines: 115
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Level_HandleCollision(this, ball, collObj): Level collision dispatcher.
   E:CATAPULTBOTTOM→Catapult_Launch+E:OPENSESAME→Trapdoor_Open N:TRAPDOOR→Trapdoor_Activate
   E:BITE→set damage 25.0 E:MACETRIGGER→activate mace N:MACE→ball bounce callback. Ends with
   CreateNoDizzy. See decomp_water_collision.c */

void __thiscall Level_HandleCollision(void *this,int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:CATAPULTBOTTOM");
  if ((iVar2 == 0) && (param_1[0x202] < 1)) {
    param_1[0x202] = 1000;
    iVar2 = AthenaList_NextIndex((int)this + 0x43b8);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x43c0) = 0;
    if (*(int *)((int)this + 0x43bc) < 1) {
      iVar3 = 0;
    }
    else {
      iVar3 = **(int **)((int)this + 0x47c4);
      *(undefined4 *)((int)this + iVar2 * 4 + 0x43c0) = 1;
    }
    while (iVar3 != 0) {
      if (*(int *)(iVar3 + 0x10d4) == *param_2) {
        *(int **)(iVar3 + 0x10ec) = param_1;
        Catapult_Launch(iVar3);
        Sound_PlayChannel(*(int *)(*(int *)((int)this + 0x878) + 0x464));
      }
      iVar1 = *(int *)((int)this + iVar2 * 4 + 0x43c0);
      if (*(int *)((int)this + 0x43bc) <= iVar1) break;
      iVar3 = *(int *)(*(int *)((int)this + 0x47c4) + iVar1 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x43c0) = iVar1 + 1;
    }
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:OPENSESAME");
  if (iVar2 == 0) {
    if (*(int *)((int)this + 0x4bec) < 1) {
      iVar2 = 0;
    }
    else {
      iVar2 = **(int **)((int)this + 0x4ff4);
    }
    Trapdoor_Open(iVar2);
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"N:TRAPDOOR");
  if (iVar2 == 0) {
    iVar2 = AthenaList_NextIndex((int)this + 0x47d0);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x47d8) = 0;
    if (*(int *)((int)this + 0x47d4) < 1) {
      iVar3 = 0;
    }
    else {
      iVar3 = **(int **)((int)this + 0x4bdc);
      *(undefined4 *)((int)this + iVar2 * 4 + 0x47d8) = 1;
    }
    while (iVar3 != 0) {
      if ((*(int *)(iVar3 + 0x10d4) == *param_2) || (*(int *)(iVar3 + 0x10dc) == *param_2)) {
        Trapdoor_Activate(iVar3);
      }
      iVar1 = *(int *)((int)this + iVar2 * 4 + 0x47d8);
      if (*(int *)((int)this + 0x47d4) <= iVar1) break;
      iVar3 = *(int *)(*(int *)((int)this + 0x4bdc) + iVar1 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x47d8) = iVar1 + 1;
    }
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:BITE");
  if (iVar2 == 0) {
    *(undefined4 *)((int)this + 0x43a8) = 0;
    *(undefined4 *)((int)this + 0x43a0) = 0x41c80000;
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"E:MACETRIGGER");
  if (iVar2 == 0) {
    iVar2 = AthenaList_NextIndex((int)this + 0x5000);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x5008) = 0;
    if (*(int *)((int)this + 0x5004) < 1) {
      iVar3 = 0;
    }
    else {
      iVar3 = **(int **)((int)this + 0x540c);
      *(undefined4 *)((int)this + iVar2 * 4 + 0x5008) = 1;
    }
    while (iVar3 != 0) {
      *(undefined1 *)(iVar3 + 0x10f0) = 1;
      iVar1 = *(int *)((int)this + iVar2 * 4 + 0x5008);
      if (*(int *)((int)this + 0x5004) <= iVar1) break;
      iVar3 = *(int *)(*(int *)((int)this + 0x540c) + iVar1 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x5008) = iVar1 + 1;
    }
  }
  iVar2 = __stricmp(*(char **)(param_2[1] + 0x864),"N:MACE");
  if (iVar2 == 0) {
    iVar2 = AthenaList_NextIndex((int)this + 0x5000);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x5008) = 0;
    if (*(int *)((int)this + 0x5004) < 1) {
      iVar3 = 0;
    }
    else {
      iVar3 = **(int **)((int)this + 0x540c);
      *(undefined4 *)((int)this + iVar2 * 4 + 0x5008) = 1;
    }
    while (iVar3 != 0) {
      if (((*(int *)(iVar3 + 0x10d4) == *param_2) && (*(int *)(iVar3 + 0x10e8) != 0x42a00000)) &&
         (*(int *)(iVar3 + 0x10f4) == 0)) {
        (**(code **)(*param_1 + 0x20))();
      }
      iVar1 = *(int *)((int)this + iVar2 * 4 + 0x5008);
      if (*(int *)((int)this + 0x5004) <= iVar1) break;
      iVar3 = *(int *)(*(int *)((int)this + 0x540c) + iVar1 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x5008) = iVar1 + 1;
    }
  }
  CreateNoDizzy(this,param_1,param_2);
  return;
}
