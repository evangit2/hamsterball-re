/*
 * Function: Scene_ComputeArenaLighting2
 * Address: 0x00423800
 * Signature: Scene_ComputeArenaLighting2(...)
 *
 * Patterns: vtable dispatch, SEH frame, scene. Calls: Scene_ComputeArenaLighting2, AthenaList_Init, AthenaList_NextIndex, ABS, AthenaList_Append, AthenaList_GetSize, AthenaList_MergeSorted, RNG_Rand. Offsets: 11, Lines: 147
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * __thiscall Scene_ComputeArenaLighting2(void *this,float *param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  bool bVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int *piVar10;
  char cVar11;
  float local_430;
  float local_428;
  undefined4 local_424;
  int local_420;
  int local_18;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca9bb;
  local_c = ExceptionList;
  if (*(int *)(*(int *)((int)this + 0x878) + 0xb28 + param_2 * 4) == 99) {
    piVar1 = *(int **)(param_2 * 0xa0 + 0x5dc + *(int *)((int)this + 0x878));
    piVar10 = (int *)0x0;
    if (piVar1[0x202] == 0) {
      ExceptionList = &local_c;
      AthenaList_Init(&local_424,0);
      iVar8 = (int)this + 0x29d4;
      local_4 = 0;
      local_428 = 9999.0;
      iVar7 = AthenaList_NextIndex(iVar8);
      *(undefined4 *)((int)this + iVar7 * 4 + 0x29dc) = 0;
      if (*(int *)((int)this + 0x29d8) < 1) {
        piVar9 = (int *)0x0;
      }
      else {
        piVar9 = (int *)**(undefined4 **)((int)this + 0x2de0);
        *(undefined4 *)((int)this + iVar7 * 4 + 0x29dc) = 1;
      }
      while (piVar9 != (int *)0x0) {
        if ((((piVar1 != piVar9) && (piVar9[0x202] == 0)) && ((char)piVar9[0xbe] == '\0')) &&
           (((char)piVar9[0x1da] != '\0' && ((float)piVar9[0x5a] < local_428)))) {
          local_428 = (float)piVar9[0x5a];
        }
        iVar2 = *(int *)((int)this + iVar7 * 4 + 0x29dc);
        if (*(int *)((int)this + 0x29d8) <= iVar2) break;
        piVar9 = *(int **)(*(int *)((int)this + 0x2de0) + iVar2 * 4);
        *(int *)((int)this + iVar7 * 4 + 0x29dc) = iVar2 + 1;
      }
      bVar6 = false;
      iVar7 = AthenaList_NextIndex(iVar8);
      *(undefined4 *)((int)this + iVar7 * 4 + 0x29dc) = 0;
      if (0 < *(int *)((int)this + 0x29d8)) {
        piVar10 = (int *)**(undefined4 **)((int)this + 0x2de0);
        *(undefined4 *)((int)this + iVar7 * 4 + 0x29dc) = 1;
      }
      while (piVar10 != (int *)0x0) {
        if (((((piVar10 != piVar1) && (piVar10[0x202] == 0)) &&
             (((char)piVar10[0xbe] == '\0' && ((char)piVar10[0x1da] != '\0')))) &&
            ((local_428 + _DAT_004d039c < (float)piVar10[0x5a] ||
             ((*(int *)(*(int *)((int)this + 0x878) + 0xb28 + piVar10[6] * 4) < 99 &&
              (ABS((float)piVar10[0x5a] - (float)piVar1[0x5a]) < (float)_DAT_004d1890)))))) &&
           (AthenaList_Append(&local_424,(int)piVar10), piVar10 == (int *)piVar1[0xa2])) {
          bVar6 = true;
        }
        iVar2 = *(int *)((int)this + iVar7 * 4 + 0x29dc);
        if (*(int *)((int)this + 0x29d8) <= iVar2) break;
        piVar10 = *(int **)(*(int *)((int)this + 0x2de0) + iVar2 * 4);
        *(int *)((int)this + iVar7 * 4 + 0x29dc) = iVar2 + 1;
      }
      iVar7 = AthenaList_GetSize((int)&local_424);
      if (iVar7 == 0) {
        AthenaList_MergeSorted(&local_424,iVar8);
      }
      else if (!bVar6) {
        piVar1[0xa2] = 0;
      }
      iVar8 = RNG_Rand(&PTR_OBJ_VTABLE,0x14,'\0');
      if ((iVar8 == 10) || (piVar1[0xa2] == 0)) {
        iVar8 = 0;
        do {
          cVar11 = '\0';
          iVar7 = AthenaList_GetSize((int)&local_424);
          iVar7 = RNG_Rand(&PTR_OBJ_VTABLE,iVar7,cVar11);
          if (iVar7 < 0) {
            iVar7 = 0;
          }
          else if (iVar7 < local_420) {
            iVar7 = *(int *)(local_18 + iVar7 * 4);
          }
          else {
            iVar7 = 0;
          }
          piVar1[0xa2] = iVar7;
        } while (((iVar7 == *(int *)(*(int *)((int)this + 0x878) + 0x5dc + param_2 * 0xa0)) ||
                 (*(float *)(iVar7 + 0x168) < _DAT_004cf368)) && (iVar8 = iVar8 + 1, iVar8 < 0x14));
        if (iVar8 == 0x14) {
          piVar1[0xa2] = 0;
        }
      }
      iVar8 = piVar1[0xa2];
      if (iVar8 == 0) {
        local_430 = 0.0;
        fVar3 = _DAT_004cf368;
        fVar4 = _DAT_004cf368;
      }
      else {
        local_430 = *(float *)(iVar8 + 0x16c);
        fVar3 = *(float *)(iVar8 + 0x164);
        fVar4 = *(float *)(iVar8 + 0x168);
      }
      fVar3 = fVar3 - (float)piVar1[0x59];
      local_430 = local_430 - (float)piVar1[0x5b];
      fVar4 = local_430 * local_430 +
              fVar3 * fVar3 + (fVar4 - (float)piVar1[0x5a]) * (fVar4 - (float)piVar1[0x5a]);
      fVar5 = _DAT_004cf368;
      if ((fVar4 < _DAT_004cf368 == (fVar4 == _DAT_004cf368)) &&
         (fVar5 = SQRT(fVar4), _DAT_004cf368 < fVar5)) {
        fVar5 = _DAT_004cf310 / fVar5;
      }
      (**(code **)(*piVar1 + 0x14))
                (fVar3 * fVar5 * _DAT_004d1bac,0,fVar5 * local_430 * _DAT_004d1bac);
      *param_1 = 0.0;
      param_1[1] = 0.0;
      param_1[2] = 0.0;
      local_4 = 0xffffffff;
      Vec3List_Free(&local_424);
    }
    else {
      *param_1 = 0.0;
      param_1[1] = 0.0;
      param_1[2] = 0.0;
    }
  }
  else {
    ExceptionList = &local_c;
    Scene_ComputeLighting(this,param_1,param_2);
  }
  ExceptionList = local_c;
  return param_1;
}
