/*
 * Function: Level_AssignTextures
 * Address: 0x004130e0
 * Signature: void __fastcall Level_AssignTextures(int param_1)
 *
 * Patterns: vtable dispatch, SEH frame, level. Calls: Level_AssignTextures, AthenaListObj_ctor, CONCAT31, Level_SetObjectTransform, MeshBuffer_dtor. Offsets: 6, Lines: 90
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Level_AssignTextures(int param_1)

{
  int iVar1;
  void *this;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int *unaff_retaddr;
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
  iVar3 = 0;
  iVar4 = 0;
  while( true ) {
    iStack_18 = iVar3 + 1;
    iVar6 = iVar5;
    if (((iVar3 < 0) || (iStack_42c <= iVar3)) ||
       (iVar1 = *(int *)(iStack_24 + iVar3 * 4), iVar1 == 0)) goto LAB_004131ac;
    if (*(int *)(iVar1 + 0x48) != 0) break;
    iVar3 = iStack_18;
    if (iVar4 == 0) {
      iVar4 = iVar1;
    }
  }
  if (*(int *)(iVar1 + 0x48) != 0) {
    iVar3 = 0;
    piVar2 = (int *)(*(int *)(param_1 + 0x878) + 0x2c8);
    goto LAB_00413187;
  }
  goto LAB_004131ac;
  while( true ) {
    iVar3 = iVar3 + 1;
    piVar2 = piVar2 + 1;
    iVar6 = iVar5;
    if (0x13 < iVar3) break;
LAB_00413187:
    if ((*piVar2 != 0) &&
       (iVar6 = iVar3, *(int *)(*(int *)(iVar1 + 0x48) + 4) == *(int *)(*piVar2 + 4))) break;
  }
LAB_004131ac:
  AthenaListObj_ctor(&uStack_858);
  puStack_8 = (undefined1 *)CONCAT31(puStack_8._1_3_,1);
  auStack_438[0] = 1;
  (**(code **)(*unaff_retaddr + 0x28))(&uStack_858);
  iStack_440 = 0;
  while (((iVar3 = iStack_440 + 1, -1 < iStack_440 && (iStack_440 < iStack_854)) &&
         (this = *(void **)(iStack_44c + iStack_440 * 4), this != (void *)0x0))) {
    if (*(int *)((int)this + 0x48) == 0) {
      iStack_440 = iVar3;
      Level_SetObjectTransform(this,iVar4);
    }
    else {
      *(undefined4 *)((int)this + 0x48) =
           *(undefined4 *)(*(int *)(param_1 + 0x878) + 0x2c8 + iVar6 * 4);
      piVar2 = (int *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2c8 + iVar6 * 4) + 0x10);
      *piVar2 = *piVar2 + 1;
      iStack_440 = iVar3;
    }
  }
  pvStack_c = (void *)((uint)pvStack_c & 0xffffff00);
  iStack_440 = iVar3;
  MeshBuffer_dtor((undefined4 *)&stack0xfffff7a4);
  pvStack_c = (void *)0xffffffff;
  MeshBuffer_dtor((undefined4 *)auStack_438);
  ExceptionList = pvStack_14;
  return;
}
