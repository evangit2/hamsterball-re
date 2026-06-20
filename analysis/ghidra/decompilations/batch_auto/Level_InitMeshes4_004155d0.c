/*
 * Function: Level_InitMeshes4
 * Address: 0x004155d0
 * Signature: Level_InitMeshes4(...)
 *
 * Patterns: vtable dispatch, SEH frame, matrix math, level. Calls: Level_InitMeshes4, AthenaListObj_ctor, Vec3_Init, Matrix_Identity, CONCAT31, Level_SetObjectTransform, MeshBuffer_dtor. Offsets: 19, Lines: 129
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Level_InitMeshes4(int param_1)

{
  int iVar1;
  void *this;
  float fVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int *unaff_retaddr;
  undefined4 uStack_86c;
  undefined4 uStack_868;
  undefined4 uStack_864;
  float fStack_860;
  undefined4 uStack_85c;
  undefined4 uStack_858;
  int iStack_854;
  int iStack_44c;
  int iStack_440;
  undefined1 auStack_438 [8];
  undefined4 local_430;
  int iStack_42c;
  int iStack_24;
  int iStack_18;
  void *pvStack_14;
  undefined1 local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9b66;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaListObj_ctor(&local_430);
  local_10 = 1;
  iVar5 = 0;
  local_4 = 0;
  (**(code **)(**(int **)(param_1 + 0x8ac) + 0x28))(&local_430);
  uStack_85c = 0;
  iVar4 = 0;
  iVar6 = 0;
  while( true ) {
    iStack_18 = iVar4 + 1;
    if (((iVar4 < 0) || (iStack_42c <= iVar4)) ||
       (iVar1 = *(int *)(iStack_24 + iVar4 * 4), iVar1 == 0)) goto LAB_00415692;
    if (*(int *)(iVar1 + 0x48) != 0) break;
    iVar4 = iStack_18;
    if (iVar6 == 0) {
      iVar6 = iVar1;
    }
  }
  if (*(int *)(iVar1 + 0x48) != 0) {
    iVar4 = 0;
    piVar3 = (int *)(*(int *)(param_1 + 0x878) + 0x2c8);
    goto LAB_00415673;
  }
  goto LAB_00415692;
  while( true ) {
    iVar5 = 0;
    iVar4 = iVar4 + 1;
    piVar3 = piVar3 + 1;
    if (0x13 < iVar4) break;
LAB_00415673:
    if ((*piVar3 != 0) &&
       (iVar5 = iVar4, *(int *)(*(int *)(iVar1 + 0x48) + 4) == *(int *)(*piVar3 + 4))) break;
  }
LAB_00415692:
  Vec3_Init(&stack0xfffff790,0x3f0f5c29,0x3f428f5c,0x3e8a3d71);
  *(float *)(iVar6 + 0x10) = fStack_860;
  fVar2 = (float)_DAT_004cf3c8;
  *(undefined4 *)(iVar6 + 4) = uStack_86c;
  *(undefined4 *)(iVar6 + 8) = uStack_868;
  *(undefined4 *)(iVar6 + 0xc) = uStack_864;
  *(bool *)(iVar6 + 0x4c) = fStack_860 != fVar2;
  Matrix_Identity((undefined4 *)&stack0xfffff790);
  Vec3_Init(&stack0xfffff790,0x3f0f5c29,0x3f428f5c,0x3e8a3d71);
  fVar2 = (float)_DAT_004cf3c8;
  *(undefined4 *)(iVar6 + 0x14) = uStack_86c;
  *(float *)(iVar6 + 0x20) = fStack_860;
  *(undefined4 *)(iVar6 + 0x18) = uStack_868;
  *(undefined4 *)(iVar6 + 0x1c) = uStack_864;
  *(bool *)(iVar6 + 0x4c) = *(float *)(iVar6 + 0x10) != fVar2;
  Matrix_Identity((undefined4 *)&stack0xfffff790);
  Vec3_Init(&stack0xfffff790,0,0,0);
  fVar2 = (float)_DAT_004cf3c8;
  *(undefined4 *)(iVar6 + 0x3c) = uStack_864;
  *(undefined4 *)(iVar6 + 0x34) = uStack_86c;
  *(undefined4 *)(iVar6 + 0x38) = uStack_868;
  *(float *)(iVar6 + 0x40) = fStack_860;
  *(bool *)(iVar6 + 0x4c) = *(float *)(iVar6 + 0x10) != fVar2;
  Matrix_Identity((undefined4 *)&stack0xfffff790);
  Vec3_Init(&stack0xfffff790,0,0,0);
  fVar2 = (float)_DAT_004cf3c8;
  *(undefined4 *)(iVar6 + 0x28) = uStack_868;
  *(undefined4 *)(iVar6 + 0x24) = uStack_86c;
  *(undefined4 *)(iVar6 + 0x2c) = uStack_864;
  *(float *)(iVar6 + 0x30) = fStack_860;
  *(bool *)(iVar6 + 0x4c) = *(float *)(iVar6 + 0x10) != fVar2;
  Matrix_Identity((undefined4 *)&stack0xfffff790);
  AthenaListObj_ctor(&uStack_858);
  puStack_8 = (undefined1 *)CONCAT31(puStack_8._1_3_,1);
  auStack_438[0] = 1;
  (**(code **)(*unaff_retaddr + 0x28))(&uStack_858);
  iStack_440 = 0;
  while (((iVar4 = iStack_440 + 1, -1 < iStack_440 && (iStack_440 < iStack_854)) &&
         (this = *(void **)(iStack_44c + iStack_440 * 4), this != (void *)0x0))) {
    if (*(int *)((int)this + 0x48) == 0) {
      iStack_440 = iVar4;
      Level_SetObjectTransform(this,iVar6);
    }
    else {
      *(undefined4 *)((int)this + 0x48) =
           *(undefined4 *)(*(int *)(param_1 + 0x878) + 0x2c8 + iVar5 * 4);
      piVar3 = (int *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2c8 + iVar5 * 4) + 0x10);
      *piVar3 = *piVar3 + 1;
      iStack_440 = iVar4;
    }
  }
  pvStack_c = (void *)((uint)pvStack_c & 0xffffff00);
  iStack_440 = iVar4;
  MeshBuffer_dtor(&uStack_85c);
  pvStack_c = (void *)0xffffffff;
  MeshBuffer_dtor((undefined4 *)auStack_438);
  ExceptionList = pvStack_14;
  return;
}
