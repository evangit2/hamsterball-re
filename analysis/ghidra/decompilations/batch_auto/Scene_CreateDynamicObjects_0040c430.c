/*
 * Function: Scene_CreateDynamicObjects
 * Address: 0x0040c430
 * Signature: Scene_CreateDynamicObjects(...)
 *
 * Patterns: vtable dispatch, SEH frame, scene. Calls: Scene_CreateDynamicObjects, AthenaList_NextIndex, Timer_Init, Gfx_ScaleX, Gfx_SetPosition, AthenaList_Append, Timer_Cleanup. Offsets: 12, Lines: 62
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_CreateDynamicObjects(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int *local_58;
  int local_54;
  undefined4 auStack_50 [17];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c95d8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar3 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar1 = *(int *)(param_1[0x22b] + 0x480);
  *(undefined4 *)(iVar1 + 0x89c + iVar3 * 4) = 0;
  if (*(int *)(iVar1 + 0x898) < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = (undefined4 *)**(undefined4 **)(iVar1 + 0xca0);
    *(undefined4 *)(iVar1 + 0x89c + iVar3 * 4) = 1;
  }
  while( true ) {
    if (puVar4 == (undefined4 *)0x0) {
      ExceptionList = local_c;
      return;
    }
    local_58 = (int *)0x0;
    local_54 = 0;
    (**(code **)(*param_1 + 0x84))(*puVar4,&local_58,&local_54,puVar4);
    if (local_58 != (int *)0x0) {
      Timer_Init(auStack_50);
      uStack_4 = 0;
      Gfx_ScaleX(_DAT_004cf44c - (float)puVar4[5]);
      Gfx_SetPosition(puVar4[1],puVar4[2],puVar4[3]);
      AthenaList_Append(param_1 + 0x335,(int)local_58);
      AthenaList_Append((void *)(*(int *)(param_1[0x22b] + 0x480) + 0x1c),(int)local_58);
      (**(code **)(*local_58 + 0x58))();
      (**(code **)(*local_58 + 0x54))(auStack_50);
      if (local_54 != 0) {
        AthenaList_Append(param_1 + 0x43b,local_54);
        AthenaList_Append((void *)(param_1[0x22c] + 0x18),local_54);
      }
      uStack_4 = 0xffffffff;
      Timer_Cleanup(auStack_50);
    }
    iVar1 = *(int *)(param_1[0x22b] + 0x480);
    iVar2 = *(int *)(iVar1 + 0x89c + iVar3 * 4);
    if (*(int *)(iVar1 + 0x898) <= iVar2) break;
    puVar4 = *(undefined4 **)(*(int *)(iVar1 + 0xca0) + iVar2 * 4);
    *(int *)(iVar1 + 0x89c + iVar3 * 4) = iVar2 + 1;
  }
  ExceptionList = local_c;
  return;
}
