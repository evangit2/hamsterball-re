/*
 * Function: Scene_CreateFlags
 * Address: 0x0040c0f0
 * Signature: Scene_CreateFlags(...)
 *
 * Patterns: allocates, SEH frame, scene. Calls: Scene_CreateFlags, AthenaList_NextIndex, __strnicmp, operator_new, WaterRipple_Ctor, AthenaList_Append. Offsets: 10, Lines: 63
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_CreateFlags(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *pvVar4;
  undefined4 *puVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c965b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar2 = AthenaList_NextIndex(*(int *)(*(int *)(param_1 + 0x8ac) + 0x480) + 0x894);
  iVar3 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
  *(undefined4 *)(iVar3 + 0x89c + iVar2 * 4) = 0;
  if (*(int *)(iVar3 + 0x898) < 1) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5 = (undefined4 *)**(undefined4 **)(iVar3 + 0xca0);
    *(undefined4 *)(iVar3 + 0x89c + iVar2 * 4) = 1;
  }
  while( true ) {
    if (puVar5 == (undefined4 *)0x0) {
      ExceptionList = local_c;
      return;
    }
    iVar3 = __strnicmp((char *)*puVar5,"FLAG",4);
    if ((iVar3 == 0) || (iVar3 = __strnicmp((char *)*puVar5,"SMALLFLAG",9), iVar3 == 0)) {
      if (*(int *)(param_1 + 0x3f18) == 0) {
        pvVar4 = operator_new(0x8c);
        local_4 = 0;
        if (pvVar4 == (void *)0x0) {
          pvVar4 = (void *)0x0;
        }
        else {
          pvVar4 = WaterRipple_Ctor(pvVar4,*(undefined4 *)(*(int *)(param_1 + 0x878) + 0x174));
        }
        local_4 = 0xffffffff;
        *(void **)(param_1 + 0x3f18) = pvVar4;
      }
      AthenaList_Append((void *)(param_1 + 0x2160),(int)puVar5);
      iVar3 = __strnicmp((char *)*puVar5,"SMALLFLAG",9);
      if (iVar3 == 0) {
        puVar5[7] = (float)puVar5[7] * _DAT_004cf3f0;
        puVar5[8] = (float)puVar5[8] * _DAT_004cf3f0;
        puVar5[9] = (float)puVar5[9] * _DAT_004cf3f0;
      }
    }
    iVar3 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
    iVar1 = *(int *)(iVar3 + 0x89c + iVar2 * 4);
    if (*(int *)(iVar3 + 0x898) <= iVar1) break;
    puVar5 = *(undefined4 **)(*(int *)(iVar3 + 0xca0) + iVar1 * 4);
    *(int *)(iVar3 + 0x89c + iVar2 * 4) = iVar1 + 1;
  }
  ExceptionList = local_c;
  return;
}
