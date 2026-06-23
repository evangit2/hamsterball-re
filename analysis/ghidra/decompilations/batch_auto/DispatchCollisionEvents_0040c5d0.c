/*
 * Function: DispatchCollisionEvents
 * Address: 0x0040c5d0
 * Signature: DispatchCollisionEvents(...)
 *
 * Patterns: vtable dispatch, SEH frame, audio, collision, rendering, ball. Calls: DispatchCollisionEvents, SetForce, ACTION, TRAJECTORY, __strnicmp, Rotator_MarkTriggered, CheckArenaUnlock, Sprite_DrawColoredRect. Offsets: 61, Lines: 362
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* DispatchCollisionEvents (shared collision handler): Dispatches ALL game events -
   N:SECRET→Rotator_MarkTriggered, N:UNLOCKSECRET→CheckArenaUnlock,
   E:NODIZZY<TIME>N</TIME>→Ball_RecordBest, E:SAFESWITCH→copy parenthesized data to ball+0xC2C,
   E:LIMIT→track arena completions per player, E:BREAK→vtable[0x20] callback,
   E:JUMP→play3D+SetForce(0.1,1)+impact=10, E:ACTION(ONCE/SCORE)→score+track,
   E:TRAJECTORY(X,Y,Z)→set trajectory, N:NOCONTROL→impact=10, N:WATER→water flag+timer=10,
   N:TARPIT→tar sound+tar flag, DROPIN→sound+score+200, PIPEBONK→random sound+score+100,
   POPOUT→sound+score+100, N:GOAL→finish race sequence, N:MOUSETRAP→deflect+rotator collision
    */

void __thiscall DispatchCollisionEvents(void *this,int *param_1,int *param_2)

{
  float *pfVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  long lVar6;
  char *pcVar7;
  uint uVar8;
  float fVar9;
  char *pcVar10;
  int iVar11;
  bool bVar12;
  float10 fVar13;
  double dVar14;
  float fVar15;
  float fVar16;
  float local_3c;
  float fStack_38;
  float fStack_34;
  undefined4 auStack_24 [6];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c96a8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  iVar4 = __strnicmp(*(char **)(param_2[1] + 0x864),"N:SECRET",8);
  if (iVar4 == 0) {
    Rotator_MarkTriggered(*(int *)(*param_2 + 0x47c));
  }
  iVar4 = __strnicmp(*(char **)(param_2[1] + 0x864),"N:UNLOCKSECRET",0xe);
  if (iVar4 == 0) {
    CheckArenaUnlock((int)this);
  }
  iVar4 = __strnicmp(*(char **)(param_2[1] + 0x864),"E:NODIZZY",9);
  if (iVar4 == 0) {
    Sprite_DrawColoredRect(&local_3c);
    local_4 = 0;
    AthenaString_Set(&local_3c,*(char **)(param_2[1] + 0x864));
    puVar5 = MWParser_ReadTag((int)&local_3c);
    while (puVar5 != (undefined4 *)0x0) {
      iVar4 = __stricmp((char *)puVar5[1],"TIME");
      if (iVar4 == 0) {
        lVar6 = _atol((char *)puVar5[2]);
        Ball_RecordBest(param_1,lVar6);
      }
      (**(code **)*puVar5)(1);
      puVar5 = MWParser_ReadTag((int)&local_3c);
    }
    local_4 = 0xffffffff;
    StreamReader_dtor(&local_3c);
  }
  iVar4 = __strnicmp(*(char **)(param_2[1] + 0x864),"E:SAFESWITCH",0xc);
  if (iVar4 == 0) {
    pcVar7 = strchr(*(char **)(param_2[1] + 0x864),0x28);
    if (pcVar7 == (char *)0x0) {
      *(undefined1 *)(param_1 + 0x30b) = 0;
    }
    else {
      iVar4 = 0xc2c - (int)pcVar7;
      do {
        cVar2 = *pcVar7;
        pcVar7[(int)param_1 + iVar4] = cVar2;
        pcVar7 = pcVar7 + 1;
      } while (cVar2 != '\0');
    }
  }
  iVar4 = __stricmp(*(char **)(param_2[1] + 0x864),"E:LIMIT");
  if (iVar4 == 0) {
    *(undefined1 *)(param_1 + 0x1da) = 0;
    *(undefined1 *)((int)param_1 + 0x2e9) = 1;
    iVar4 = *(int *)((int)this + 0x878);
    if (((*(char *)(iVar4 + 0x237) != '\0') && (param_1[6] != -1)) &&
       (iVar11 = param_1[5], *(char *)(iVar11 + 0x47c4) == '\0')) {
      if (((*(char *)(iVar4 + 0x5d7) == '\0') && (param_1[6] != 0)) &&
         ((*(char *)(*(int *)(iVar4 + 0x5dc) + 0x768) != '\0' &&
          (*(char *)(*(int *)(iVar4 + 0x5dc) + 0x2f8) == '\0')))) {
        *(int *)(iVar11 + 0x47b4) = *(int *)(iVar11 + 0x47b4) + 1;
      }
      if (((*(char *)(*(int *)((int)this + 0x878) + 0x677) == '\0') && (param_1[6] != 1)) &&
         ((iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x67c), *(char *)(iVar4 + 0x768) != '\0'
          && (*(char *)(iVar4 + 0x2f8) == '\0')))) {
        *(int *)(iVar11 + 0x47b8) = *(int *)(iVar11 + 0x47b8) + 1;
      }
      if ((((*(char *)(*(int *)((int)this + 0x878) + 0x717) == '\0') && (param_1[6] != 2)) &&
          (iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x71c), *(char *)(iVar4 + 0x768) != '\0'))
         && (*(char *)(iVar4 + 0x2f8) == '\0')) {
        *(int *)(iVar11 + 0x47bc) = *(int *)(iVar11 + 0x47bc) + 1;
      }
      if (((*(char *)(*(int *)((int)this + 0x878) + 0x7b7) == '\0') && (param_1[6] != 3)) &&
         ((iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x7bc), *(char *)(iVar4 + 0x768) != '\0'
          && (*(char *)(iVar4 + 0x2f8) == '\0')))) {
        *(int *)(iVar11 + 0x47c0) = *(int *)(iVar11 + 0x47c0) + 1;
      }
    }
  }
  iVar4 = __stricmp(*(char **)(param_2[1] + 0x864),"E:BREAK");
  if (iVar4 == 0) {
    (**(code **)(*param_1 + 0x20))();
  }
  iVar4 = __stricmp(*(char **)(param_2[1] + 0x864),"E:JUMP");
  if ((iVar4 == 0) && (param_1[0x1f7] < 1)) {
    fStack_34 = (float)param_1[0x5b];
    local_3c = (float)param_1[0x59];
    fStack_38 = (float)param_1[0x5a];
    Sound_Play3D(*(void **)(*(int *)((int)this + 0x878) + 0x49c),local_3c,fStack_38,fStack_34);
    param_1[0x1f7] = 10;
    param_1[0xa7] = 0x3b03126f;
    param_1[0xa8] = 1;
    param_1[0x202] = 10;
    Ball_RecordBest(param_1,200);
  }
  iVar4 = __strnicmp(*(char **)(param_2[1] + 0x864),"E:ACTION",8);
  if (iVar4 == 0) {
    Sprite_DrawColoredRect(&local_3c);
    local_4 = 1;
    AthenaString_Set(&local_3c,*(char **)(param_2[1] + 0x864));
    puVar5 = MWParser_ReadTag((int)&local_3c);
    while (puVar5 != (undefined4 *)0x0) {
      iVar4 = __stricmp((char *)puVar5[1],"ONCE");
      if ((iVar4 == 0) &&
         ((iVar4 = __stricmp((char *)puVar5[2],"TRUE"), iVar4 == 0 ||
          (iVar4 = __stricmp((char *)puVar5[2],(char *)&PTR_DAT_004cf870), iVar4 == 0)))) {
        uVar8 = AthenaList_ContainsValue(param_1 + 0xcb,param_2[1]);
        if ((char)uVar8 != '\0') break;
        AthenaList_Append(param_1 + 0xcb,param_2[1]);
      }
      iVar4 = __stricmp((char *)puVar5[1],"SCORE");
      if (iVar4 == 0) {
        lVar6 = _atol((char *)puVar5[2]);
        fVar13 = Difficulty_GetTimeModifier(*(void **)((int)this + 0x878),(float)lVar6);
        *(float *)(param_1[6] * 0xa0 + 0x5e4 + *(int *)((int)this + 0x878)) =
             (float)(fVar13 + (float10)*(float *)(param_1[6] * 0xa0 + 0x5e4 +
                                                 *(int *)((int)this + 0x878)));
        __ftol2();
        pcVar7 = (char *)AthenaString_Format(0x4f7448,&DAT_004cf500);
        Ball_SetName(param_1,pcVar7);
      }
      puVar5 = MWParser_ReadTag((int)&local_3c);
    }
    local_4 = 0xffffffff;
    StreamReader_dtor(&local_3c);
  }
  fVar9 = (float)__strnicmp(*(char **)(param_2[1] + 0x864),"E:TRAJECTORY",0xc);
  if (fVar9 == 0.0) {
    local_3c = fVar9;
    fStack_38 = fVar9;
    fStack_34 = fVar9;
    Sprite_DrawColoredRect(auStack_24);
    local_4 = 2;
    AthenaString_Set(auStack_24,*(char **)(param_2[1] + 0x864));
    puVar5 = MWParser_ReadTag((int)auStack_24);
    while (puVar5 != (undefined4 *)0x0) {
      iVar4 = __stricmp((char *)puVar5[1],"X");
      if (iVar4 == 0) {
        dVar14 = _atof((char *)puVar5[2]);
        local_3c = (float)dVar14;
      }
      iVar4 = __stricmp((char *)puVar5[1],"Y");
      if (iVar4 == 0) {
        dVar14 = _atof((char *)puVar5[2]);
        fStack_38 = (float)dVar14;
      }
      iVar4 = __stricmp((char *)puVar5[1],"Z");
      if (iVar4 == 0) {
        dVar14 = _atof((char *)puVar5[2]);
        fStack_34 = (float)dVar14;
      }
      (**(code **)*puVar5)(1);
      puVar5 = MWParser_ReadTag((int)auStack_24);
    }
    iVar4 = param_1[0x69];
    if ((float *)(iVar4 + 0xca4) != &local_3c) {
      *(float *)(iVar4 + 0xca4) = local_3c;
      *(float *)(iVar4 + 0xca8) = fStack_38;
      *(float *)(iVar4 + 0xcac) = fStack_34;
    }
    local_4 = 0xffffffff;
    StreamReader_dtor(auStack_24);
  }
  iVar4 = __stricmp(*(char **)(param_2[1] + 0x864),"N:NOCONTROL");
  if (iVar4 == 0) {
    param_1[0x202] = 10;
  }
  iVar4 = __stricmp(*(char **)(param_2[1] + 0x864),"N:WATER");
  if (iVar4 == 0) {
    *(undefined1 *)((int)param_1 + 0x2d5) = 1;
    param_1[0xb6] = 10;
  }
  iVar4 = __stricmp(*(char **)(param_2[1] + 0x864),"N:TARPIT");
  if (iVar4 == 0) {
    if ((char)param_1[0xb3] == '\0') {
      fStack_38 = (float)param_1[0x5a];
      local_3c = (float)param_1[0x59];
      param_1[0xb4] = param_1[0x5a];
      fStack_34 = (float)param_1[0x5b];
      Sound_Play3D(*(void **)(*(int *)((int)this + 0x878) + 0x484),local_3c,fStack_38,fStack_34);
    }
    *(undefined1 *)(param_1 + 0xb3) = 1;
    *(undefined1 *)(param_1 + 0x1da) = 0;
  }
  iVar4 = __stricmp((char *)(*(int *)(param_2[1] + 0x864) + 2),"DROPIN");
  if (iVar4 == 0) {
    iVar4 = param_1[0x69];
    fVar9 = *(float *)(iVar4 + 0xcac) * *(float *)(iVar4 + 0xcac) +
            *(float *)(iVar4 + 0xca8) * *(float *)(iVar4 + 0xca8) +
            *(float *)(iVar4 + 0xca4) * *(float *)(iVar4 + 0xca4);
    if (((fVar9 < _DAT_004cf368 == (fVar9 == _DAT_004cf368)) && (_DAT_004cf48c < SQRT(fVar9))) &&
       (param_1[0x1f2] < 1)) {
      Sound_PlayChannel(*(int *)(*(int *)((int)this + 0x878) + 0x460));
      param_1[0x1f2] = 0x32;
      Ball_RecordBest(param_1,200);
    }
  }
  iVar4 = __stricmp((char *)(*(int *)(param_2[1] + 0x864) + 2),"PIPEBONK");
  if ((iVar4 == 0) && (param_1[500] < 1)) {
    RNG_Rand(&PTR_OBJ_VTABLE,5,'\0');
    fVar9 = (float)param_1[0x59];
    fVar15 = (float)param_1[0x5a];
    fVar16 = (float)param_1[0x5b];
    local_3c = fVar9;
    fStack_38 = fVar15;
    fStack_34 = fVar16;
    iVar4 = RNG_Rand(&PTR_OBJ_VTABLE,3,'\0');
    Sound_Play3D(*(void **)(*(int *)((int)this + 0x878) + 0x46c + iVar4 * 4),fVar9,fVar15,fVar16);
    param_1[500] = 10;
    Ball_RecordBest(param_1,100);
  }
  iVar4 = __stricmp((char *)(*(int *)(param_2[1] + 0x864) + 2),"POPOUT");
  if ((iVar4 == 0) && (param_1[499] < 1)) {
    Sound_PlayChannel(*(int *)(*(int *)((int)this + 0x878) + 0x468));
    param_1[499] = 0x32;
    Ball_RecordBest(param_1,100);
  }
  iVar4 = __stricmp((char *)(*(int *)(param_2[1] + 0x864) + 2),(char *)&PTR_DAT_004cf80c);
  if ((iVar4 == 0) && (param_1[0x1f5] < 1)) {
    Sound_PlayChannel(*(int *)(*(int *)((int)this + 0x878) + 0x4cc));
    param_1[0x1f5] = 0x32;
  }
  iVar4 = 7;
  bVar12 = true;
  pcVar7 = *(char **)(param_2[1] + 0x864);
  pcVar10 = "N:GOAL";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar12 = *pcVar7 == *pcVar10;
    pcVar7 = pcVar7 + 1;
    pcVar10 = pcVar10 + 1;
  } while (bVar12);
  if (((bVar12) && ((char)param_1[0x53] == '\0')) && ((char)param_1[0x1da] != '\0')) {
    if (*(char *)((int)this + 0xcd0) == '\0') {
      *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Reach Goal 1";
      Audio_PlayMusic(*(void **)(*(int *)((int)this + 0x878) + 0x53c),"Goal!");
      iVar4 = *(int *)((int)this + 0x878);
      *(undefined1 *)((int)this + 0xcd0) = 1;
      if (((*(char *)(*(int *)(iVar4 + 0x220) + 0x11) != '\0') && (*(char *)(iVar4 + 0x234) == '\0')
          ) && (*(int *)(iVar4 + 0x90c) != 0)) {
        *(undefined4 *)(*(int *)(iVar4 + 0x90c) + 0x524) = *(undefined4 *)(iVar4 + 0x5e8);
      }
      *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Reach Goal 2";
    }
    *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Reach Goal 3";
    iVar4 = param_1[6] * 0xa0 + *(int *)((int)this + 0x878);
    if (*(char *)(iVar4 + 0x5d6) == '\0') {
      *(undefined4 *)(iVar4 + 0x5fc) = 1;
    }
    *(undefined1 *)(param_1[6] * 0xa0 + 0x5d6 + *(int *)((int)this + 0x878)) = 1;
    *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Reach Goal 4";
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x294) + 4) =
         *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x29c) + 4);
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x2b4) + 4) =
         *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 700) + 4);
    *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Reach Goal 5";
    iVar4 = *(int *)((int)this + 0x878);
    if (*(char *)(iVar4 + 0x236) != '\0') {
      *(undefined4 *)(*(int *)(iVar4 + 0x294) + 4) = *(undefined4 *)(*(int *)(iVar4 + 0x2a4) + 4);
      *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x2b4) + 4) =
           *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x2c4) + 4);
    }
    *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Reach Goal 6";
    if (*(char *)(*(int *)((int)this + 0x878) + param_1[6] * -0xa0 + 0x690) == '\0') {
      *(undefined1 *)(param_1[6] * 0xa0 + 0x5f0 + *(int *)((int)this + 0x878)) = 1;
    }
    *(char **)(*(int *)((int)this + 0x878) + 0x210) = "Update";
  }
  iVar4 = 0xc;
  bVar12 = true;
  pcVar7 = *(char **)(param_2[1] + 0x864);
  pcVar10 = "N:MOUSETRAP";
  do {
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    bVar12 = *pcVar7 == *pcVar10;
    pcVar7 = pcVar7 + 1;
    pcVar10 = pcVar10 + 1;
  } while (bVar12);
  if (bVar12) {
    RNG_Rand(&PTR_OBJ_VTABLE,0x32,'\x01');
    RNG_Rand(&PTR_OBJ_VTABLE,0x32,'\x01');
    iVar4 = param_1[0x69];
    pfVar1 = (float *)(iVar4 + 0xca4);
    if (&local_3c != pfVar1) {
      local_3c = *pfVar1;
      fStack_38 = *(float *)(iVar4 + 0xca8);
      fStack_34 = *(float *)(iVar4 + 0xcac);
    }
    Vec3_NormalizeAndScale(&local_3c,1.0);
    fStack_34 = fStack_34 * _DAT_004cf370;
    if (pfVar1 != &local_3c) {
      *pfVar1 = local_3c * _DAT_004cf370;
      *(undefined4 *)(iVar4 + 0xca8) = 0x41700000;
      *(float *)(iVar4 + 0xcac) = fStack_34;
    }
    iVar4 = AthenaList_NextIndex((int)this + 0x1930);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x1938) = 0;
    if (*(int *)((int)this + 0x1934) < 1) {
      iVar11 = 0;
    }
    else {
      iVar11 = **(int **)((int)this + 0x1d3c);
      *(undefined4 *)((int)this + iVar4 * 4 + 0x1938) = 1;
    }
    while (iVar11 != 0) {
      if (*(int *)(*param_2 + 0x47c) == *(int *)(iVar11 + 0x10d4)) {
        Rotator_PlayCollisionSound(iVar11);
        uVar8 = AthenaList_ContainsValue((void *)((int)this + 0x2578),iVar11);
        if ((char)uVar8 != '\0') {
          ExceptionList = pvStack_c;
          return;
        }
        AthenaList_Append((void *)((int)this + 0x2578),iVar11);
        ExceptionList = pvStack_c;
        return;
      }
      iVar3 = *(int *)((int)this + iVar4 * 4 + 0x1938);
      if (*(int *)((int)this + 0x1934) <= iVar3) {
        ExceptionList = pvStack_c;
        return;
      }
      iVar11 = *(int *)(*(int *)((int)this + 0x1d3c) + iVar3 * 4);
      *(int *)((int)this + iVar4 * 4 + 0x1938) = iVar3 + 1;
    }
  }
  ExceptionList = pvStack_c;
  return;
}
