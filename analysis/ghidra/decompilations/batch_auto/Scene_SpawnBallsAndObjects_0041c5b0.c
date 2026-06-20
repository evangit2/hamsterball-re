/*
 * Function: Scene_SpawnBallsAndObjects
 * Address: 0x0041c5b0
 * Signature: Scene_SpawnBallsAndObjects(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, audio, ball, scene, level. Calls: Scene_SpawnBallsAndObjects, AthenaList_GetSize, AthenaList_NextIndex, AthenaString_Format, AthenaHashTable_Lookup, RNG_Rand, operator_new, Ball_ctor2. Offsets: 17, Lines: 163
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Scene_SpawnBallsAndObjects: Level startup. 1)For each start entry: lookup "START%d-%d" in hash
   table for position, Ball_ctor2, set radius=26, max_speed=5, gravity=0.5 2)Scan SAFESPOT/SAFEPOS
   3)If demo/tournament: CreateBadBall+CreateMouseTrap 4)CreateFlags,CreateSigns,CreateDynObjects.
   See docs/LEVEL_RENDER_DECOMP.md */

void __fastcall Scene_SpawnBallsAndObjects(int *param_1)

{
  int *piVar1;
  float fVar2;
  undefined4 uVar3;
  int iVar4;
  char *pcVar5;
  float *pfVar6;
  int iVar7;
  void *this;
  int iVar8;
  Ball *this_00;
  undefined4 *puVar9;
  undefined1 *puVar10;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  undefined4 local_3c [3];
  undefined4 local_30 [3];
  undefined4 local_24 [3];
  undefined4 local_18 [3];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca12b;
  local_c = ExceptionList;
  piVar1 = param_1 + 0xd8b;
  ExceptionList = &local_c;
  uVar3 = AthenaList_GetSize((int)piVar1);
  *(undefined4 *)(*(int *)(param_1[0x21e] + 0x178) + 0x850) = uVar3;
  iVar4 = AthenaList_NextIndex((int)piVar1);
  param_1[iVar4 + 0xd8d] = 0;
  if (param_1[0xd8c] < 1) {
    iVar8 = 0;
  }
  else {
    iVar8 = *(int *)param_1[0xe8e];
    param_1[iVar4 + 0xd8d] = 1;
  }
  while (iVar8 != 0) {
    puVar10 = (undefined1 *)0x0;
    pcVar5 = (char *)AthenaString_Format(0x4f7448,(byte *)"START%d-%d");
    pfVar6 = (float *)AthenaHashTable_Lookup((void *)param_1[0x22b],local_3c,pcVar5,puVar10);
    if (&local_54 != pfVar6) {
      local_54 = *pfVar6;
      local_50 = pfVar6[1];
      local_4c = pfVar6[2];
    }
    if (*(char *)(param_1[0x21e] + 0x237) != '\0') {
      puVar10 = (undefined1 *)0x0;
      pcVar5 = (char *)AthenaString_Format(0x4f7448,(byte *)"START%d-%d");
      pfVar6 = (float *)AthenaHashTable_Lookup((void *)param_1[0x22b],local_30,pcVar5,puVar10);
      if (&local_54 != pfVar6) {
        local_54 = *pfVar6;
        local_50 = pfVar6[1];
        local_4c = pfVar6[2];
      }
    }
    iVar7 = AthenaList_GetSize((int)piVar1);
    if ((iVar7 == 1) &&
       ((((iVar7 = *(int *)(*(int *)(param_1[0x21e] + 0x220) + 8), iVar7 == 5 || (iVar7 == 0xb)) ||
         (iVar7 == 0xc)) || (iVar7 == 0xe)))) {
      iVar7 = RNG_Rand(&PTR_OBJ_VTABLE,2,'\0');
      puVar10 = (undefined1 *)0x0;
      if (iVar7 == 0) {
        pcVar5 = (char *)AthenaString_Format(0x4f7448,(byte *)"START2-1");
        puVar9 = local_24;
      }
      else {
        pcVar5 = (char *)AthenaString_Format(0x4f7448,(byte *)"START2-2");
        puVar9 = local_18;
      }
      pfVar6 = (float *)AthenaHashTable_Lookup((void *)param_1[0x22b],puVar9,pcVar5,puVar10);
      if (&local_54 != pfVar6) {
        local_54 = *pfVar6;
        local_50 = pfVar6[1];
        local_4c = pfVar6[2];
      }
    }
    iVar7 = AthenaList_GetSize((int)piVar1);
    if ((iVar7 == 1) &&
       (((AthenaHashTable_Lookup((void *)param_1[0x22b],&local_48,"START-DEBUG",(undefined1 *)0x0),
         local_48 != _DAT_004cf368 || (local_44 != _DAT_004cf368)) || (local_40 != _DAT_004cf368))))
    {
      local_54 = local_48;
      local_50 = local_44;
      local_4c = local_40;
    }
    this = operator_new(0xc60);
    this_00 = (Ball *)0x0;
    local_4 = 0;
    if (this != (void *)0x0) {
      this_00 = Ball_ctor2(this,(int)param_1);
    }
    local_4 = 0xffffffff;
    (**(code **)(this_00->dwVtable + 4))();
    iVar7 = *(int *)(iVar8 + 0x14) + 0x2ef;
    Ball_SetTrajectory(this_00,&PTR_Vec3_dtor_004cf300,(float)param_1[iVar7 * 5 + 1],
                       (float)param_1[iVar7 * 5 + 2],(float)param_1[iVar7 * 5 + 3],
                       (float)param_1[iVar7 * 5 + 4]);
    iVar7 = *(int *)(iVar8 + 0x14);
    fVar2 = local_50 + _DAT_004d0434;
    this_00->flPos_z = local_4c;
    this_00->flPos_y = fVar2;
    this_00->nPlayer_index = iVar7;
    this_00->flGravity_scale = 0.5;
    *(undefined4 *)&this_00->field_0x27c = 0x3dcccccd;
    this_00->flRadius = 26.0;
    *(undefined4 *)&this_00->field_0x1a0 = 0x3f866666;
    this_00->flMax_speed_float = 5.0;
    this_00->flPos_x = local_54;
    AthenaList_Append(param_1 + 0xa75,(int)this_00);
    this_00->bIs_falling = 0;
    this_00->field_0x769 = 1;
    *(Ball **)(iVar8 + 0x10) = this_00;
    iVar7 = param_1[iVar4 + 0xd8d];
    if (param_1[0xd8c] <= iVar7) break;
    iVar8 = *(int *)(param_1[0xe8e] + iVar7 * 4);
    param_1[iVar4 + 0xd8d] = iVar7 + 1;
  }
  iVar8 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar4 = *(int *)(param_1[0x22b] + 0x480);
  puVar9 = (undefined4 *)0x0;
  *(undefined4 *)(iVar4 + 0x89c + iVar8 * 4) = 0;
  if (0 < *(int *)(iVar4 + 0x898)) {
    puVar9 = (undefined4 *)**(undefined4 **)(iVar4 + 0xca0);
    *(undefined4 *)(iVar4 + 0x89c + iVar8 * 4) = 1;
  }
  while (puVar9 != (undefined4 *)0x0) {
    iVar4 = __strnicmp((char *)*puVar9,"SAFESPOT",8);
    if ((iVar4 == 0) || (iVar4 = __strnicmp((char *)*puVar9,"SAFEPOS",7), iVar4 == 0)) {
      AthenaList_Append(param_1 + 0x546,(int)puVar9);
    }
    iVar4 = *(int *)(param_1[0x22b] + 0x480);
    iVar7 = *(int *)(iVar4 + 0x89c + iVar8 * 4);
    if (*(int *)(iVar4 + 0x898) <= iVar7) break;
    puVar9 = *(undefined4 **)(*(int *)(iVar4 + 0xca0) + iVar7 * 4);
    *(int *)(iVar4 + 0x89c + iVar8 * 4) = iVar7 + 1;
  }
  if ((*(int *)(param_1[0x21e] + 0x23c) != 0) || (*(char *)(param_1[0x21e] + 0x237) != '\0')) {
    CreateBadBall((int)param_1);
  }
  if ((*(int *)(param_1[0x21e] + 0x23c) != 0) || (*(char *)(param_1[0x21e] + 0x237) != '\0')) {
    CreateMouseTrap((int)param_1);
  }
  CreateSecretObjects((int)param_1);
  Scene_CreateFlags((int)param_1);
  Scene_CreateSigns((int)param_1);
  Scene_CreateDynamicObjects(param_1);
  ExceptionList = local_c;
  return;
}
