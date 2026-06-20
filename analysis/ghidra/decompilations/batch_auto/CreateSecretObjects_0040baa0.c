/*
 * Function: CreateSecretObjects
 * Address: 0x0040baa0
 * Signature: CreateSecretObjects(...)
 *
 * Patterns: allocates, SEH frame. Calls: CreateSecretObjects, AthenaList_NextIndex, __strnicmp, operator_new, Secret_ctor, AthenaList_Append. Offsets: 19, Lines: 90
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* CreateSecretObjects: Factory for SECRET and SECRETUNLOCK game objects. Calls
   Rotator_MarkTriggered for N:SECRET, CheckArenaUnlock for N:UNLOCKSECRET. */

void __fastcall CreateSecretObjects(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *pvVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  char *_Str1;
  undefined4 uVar7;
  void *pvVar8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9606;
  local_c = ExceptionList;
  iVar3 = *(int *)(param_1 + 0x878);
  if (((*(int *)(iVar3 + 0x23c) != 0) && (*(char *)(iVar3 + 0x234) == '\0')) &&
     (*(char *)(*(int *)(iVar3 + 0x220) + 0x11) == '\0')) {
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
    while (puVar5 != (undefined4 *)0x0) {
      _Str1 = (char *)*puVar5;
      uVar6 = 0x40bb5f;
      iVar3 = __strnicmp(_Str1,"SECRETUNLOCK",0xc);
      if (iVar3 == 0) {
        uVar7 = 0x40bb70;
        pvVar4 = operator_new(0x10ec);
        local_4 = 0;
        if (pvVar4 == (void *)0x0) {
LAB_0040bc09:
          pvVar4 = (void *)0x0;
        }
        else {
          pvVar8 = *(void **)(*(int *)(param_1 + 0x878) + 0x580);
LAB_0040bb9b:
          if ((undefined4 *)&stack0xffffffcc != puVar5 + 1) {
            uVar6 = puVar5[1];
            _Str1 = (char *)puVar5[2];
            uVar7 = puVar5[3];
          }
          pvVar4 = Secret_ctor(pvVar4,param_1,uVar6,_Str1,uVar7,pvVar8);
        }
        local_4 = 0xffffffff;
        AthenaList_Append((void *)(param_1 + 0xcd4),(int)pvVar4);
        AthenaList_Append((void *)(*(int *)(*(int *)(param_1 + 0x8ac) + 0x480) + 0x1c),(int)pvVar4);
        AthenaList_Append((void *)(*(int *)(param_1 + 0x8b0) + 0x18),*(int *)((int)pvVar4 + 0x10e0))
        ;
        AthenaList_Append((void *)(param_1 + 0x2578),(int)pvVar4);
      }
      else {
        _Str1 = (char *)*puVar5;
        uVar6 = 0x40bbcf;
        iVar3 = __strnicmp(_Str1,"SECRET",6);
        if (iVar3 == 0) {
          uVar7 = 0x40bbe0;
          pvVar4 = operator_new(0x10ec);
          local_4 = 1;
          if (pvVar4 == (void *)0x0) goto LAB_0040bc09;
          pvVar8 = *(void **)(*(int *)(param_1 + 0x878) + 0x57c);
          goto LAB_0040bb9b;
        }
      }
      iVar3 = *(int *)(*(int *)(param_1 + 0x8ac) + 0x480);
      iVar1 = *(int *)(iVar3 + 0x89c + iVar2 * 4);
      if (*(int *)(iVar3 + 0x898) <= iVar1) {
        ExceptionList = local_c;
        return;
      }
      puVar5 = *(undefined4 **)(*(int *)(iVar3 + 0xca0) + iVar1 * 4);
      *(int *)(iVar3 + 0x89c + iVar2 * 4) = iVar1 + 1;
    }
  }
  ExceptionList = local_c;
  return;
}
