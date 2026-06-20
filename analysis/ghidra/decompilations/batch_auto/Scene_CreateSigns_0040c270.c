/*
 * Function: Scene_CreateSigns
 * Address: 0x0040c270
 * Signature: void __fastcall Scene_CreateSigns(int param_1)
 *
 * Patterns: allocates, SEH frame, scene. Calls: Scene_CreateSigns, AthenaList_NextIndex, __strnicmp, operator_new, StandsTipper_Ctor, __stricmp, AthenaList_Append. Offsets: 17, Lines: 80
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_CreateSigns(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *this;
  void *pvVar4;
  undefined4 *puVar5;
  undefined4 in_stack_ffffffc4;
  undefined4 in_stack_ffffffc8;
  undefined4 uVar6;
  char *_Str1;
  undefined4 uVar7;
  undefined4 uVar8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c967b;
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
    pvVar4 = (void *)0x0;
    _Str1 = (char *)*puVar5;
    uVar6 = 0x40c2ef;
    iVar3 = __strnicmp(_Str1,"SIGN",4);
    if (iVar3 == 0) {
      uVar8 = 0x10fc;
      uVar7 = 0x40c304;
      this = operator_new(0x10fc);
      local_4 = 0;
      if (this != (void *)0x0) {
        if ((undefined4 *)&stack0xffffffd0 != puVar5 + 4) {
          _Str1 = (char *)puVar5[4];
          uVar7 = puVar5[5];
          uVar8 = puVar5[6];
        }
        if ((undefined4 *)&stack0xffffffc4 != puVar5 + 1) {
          in_stack_ffffffc4 = puVar5[1];
          in_stack_ffffffc8 = puVar5[2];
          uVar6 = puVar5[3];
        }
        pvVar4 = StandsTipper_Ctor(this,param_1,*(int *)(*(int *)(param_1 + 0x878) + 0x58c),
                                   *(int *)(*(int *)(param_1 + 0x878) + 0x590),in_stack_ffffffc4,
                                   in_stack_ffffffc8,uVar6,_Str1,uVar7,uVar8);
      }
      local_4 = 0xffffffff;
      iVar3 = __stricmp((char *)*puVar5,"SIGN-TARPIT");
      if (iVar3 == 0) {
        *(undefined4 *)((int)pvVar4 + 0x10f8) = *(undefined4 *)(*(int *)(param_1 + 0x878) + 0x27c);
      }
      AthenaList_Append((void *)(param_1 + 0xcd4),(int)pvVar4);
      AthenaList_Append((void *)(*(int *)(*(int *)(param_1 + 0x8ac) + 0x480) + 0x1c),(int)pvVar4);
      AthenaList_Append((void *)(*(int *)(param_1 + 0x8b0) + 0x18),*(int *)((int)pvVar4 + 0x10ec));
      AthenaList_Append((void *)(param_1 + 0x2578),(int)pvVar4);
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
