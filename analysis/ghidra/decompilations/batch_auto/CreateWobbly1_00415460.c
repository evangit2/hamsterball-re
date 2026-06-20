/*
 * Function: CreateWobbly1
 * Address: 0x00415460
 * Signature: void __thiscall
CreateWobbly1(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
 *
 * Patterns: allocates, SEH frame, level. Calls: CreateWobbly1, __strnicmp, operator_new, GameLevel_ctor, AthenaList_Append, CreatePlatformOrStands. Offsets: 4, Lines: 48
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
CreateWobbly1(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  undefined4 uVar2;
  void *pvVar3;
  undefined4 uVar4;
  char *pcVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9c6b;
  local_c = ExceptionList;
  pvVar3 = (void *)0x0;
  uVar2 = 0;
  uVar4 = 0x415491;
  ExceptionList = &local_c;
  pcVar5 = param_1;
  iVar1 = __strnicmp(param_1,"WOBBLY1",7);
  if (iVar1 == 0) {
    uVar2 = 0x4154a2;
    pvVar3 = operator_new(0x1524);
    local_4 = 0;
    if (pvVar3 == (void *)0x0) {
      pvVar3 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar4 = *(undefined4 *)(param_4 + 4);
        pcVar5 = *(char **)(param_4 + 8);
        uVar2 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar3 = GameLevel_ctor(pvVar3,this,uVar4,pcVar5,uVar2,*(void **)((int)this + 0x47e0));
    }
    uVar2 = *(undefined4 *)((int)pvVar3 + 0x10d4);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar3);
  }
  *param_2 = pvVar3;
  *param_3 = uVar2;
  if (pvVar3 == (void *)0x0) {
    CreatePlatformOrStands(this,param_1,param_2,param_3,param_4);
  }
  ExceptionList = local_c;
  return;
}
