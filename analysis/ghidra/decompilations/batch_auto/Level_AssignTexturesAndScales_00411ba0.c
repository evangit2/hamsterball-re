/*
 * Function: Level_AssignTexturesAndScales
 * Address: 0x00411ba0
 * Signature: Level_AssignTexturesAndScales(...)
 *
 * Patterns: vtable dispatch, SEH frame, level. Calls: Level_AssignTexturesAndScales, AthenaListObj_ctor, CONCAT31, MeshBuffer_dtor. Offsets: 10, Lines: 163
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Level_AssignTexturesAndScales(void *this,int *param_1)

{
  int *piVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined4 local_430;
  int iStack_42c;
  int iStack_24;
  int iStack_18;
  void *local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c99db;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaListObj_ctor(&local_430);
  local_4 = 0;
  local_10 = (void *)CONCAT31(local_10._1_3_,1);
  (**(code **)(*param_1 + 0x28))(&local_430);
  iStack_18 = 0;
  while (((iVar5 = iStack_18 + 1, -1 < iStack_18 && (iStack_18 < iStack_42c)) &&
         (iVar3 = *(int *)(iStack_24 + iStack_18 * 4), iVar3 != 0))) {
    if (*(int *)(iVar3 + 0x48) == 0) {
      if (*(float *)(iVar3 + 4) <= *(float *)(iVar3 + 8)) {
        fVar2 = *(float *)(iVar3 + 8);
      }
      else {
        fVar2 = *(float *)(iVar3 + 4);
      }
      if (fVar2 <= *(float *)(iVar3 + 0xc)) {
        fVar2 = *(float *)(iVar3 + 0xc);
      }
      else if (*(float *)(iVar3 + 4) <= *(float *)(iVar3 + 8)) {
        fVar2 = *(float *)(iVar3 + 8);
      }
      else {
        fVar2 = *(float *)(iVar3 + 4);
      }
      if (fVar2 <= (float)_DAT_004cf3e0) {
        *(undefined4 *)(iVar3 + 4) = 0x3ed70a3d;
        *(undefined4 *)(iVar3 + 8) = 0x3ebd70a4;
        *(undefined4 *)(iVar3 + 0xc) = 0x3e19999a;
        *(undefined4 *)(iVar3 + 0x14) = 0x3ed70a3d;
        *(undefined4 *)(iVar3 + 0x18) = 0x3ebd70a4;
        *(undefined4 *)(iVar3 + 0x1c) = 0x3e19999a;
        iStack_18 = iVar5;
      }
      else {
        *(undefined4 *)(iVar3 + 4) = 0x3f570a3d;
        *(undefined4 *)(iVar3 + 8) = 0x3f3d70a4;
        *(undefined4 *)(iVar3 + 0xc) = 0x3e99999a;
        *(undefined4 *)(iVar3 + 0x14) = 0x3f570a3d;
        *(undefined4 *)(iVar3 + 0x18) = 0x3f3d70a4;
        *(undefined4 *)(iVar3 + 0x1c) = 0x3e99999a;
        iStack_18 = iVar5;
      }
    }
    else {
      iVar7 = 0x2cc;
      iVar6 = 2;
      do {
        if (iVar6 != 8) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + -4 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 7) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 6) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 4 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 5) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 8 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 4) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0xc + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 3) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x10 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 2) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x14 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 1) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x18 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != 0) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x1c + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        if (iVar6 != -1) {
          iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0x20 + iVar7);
          if ((iVar4 != 0) && (*(int *)(*(int *)(iVar3 + 0x48) + 4) == *(int *)(iVar4 + 4))) {
            piVar1 = (int *)(*(int *)(*(int *)((int)this + 0x878) + 0x2e0) + 0x10);
            *piVar1 = *piVar1 + 1;
            *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(*(int *)((int)this + 0x878) + 0x2e0);
          }
        }
        iVar4 = iVar6 + 8;
        iVar7 = iVar7 + 0x28;
        iVar6 = iVar6 + 10;
        iStack_18 = iVar5;
      } while (iVar4 < 0x14);
    }
  }
  puStack_8 = (undefined1 *)0xffffffff;
  iStack_18 = iVar5;
  MeshBuffer_dtor((undefined4 *)&stack0xfffffbcc);
  ExceptionList = local_10;
  return;
}
