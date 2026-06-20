/*
 * Function: Scene_CreateObject_Gear
 * Address: 0x00418760
 * Signature: void __thiscall
Scene_CreateObject_Gear
          (void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
 *
 * Patterns: allocates, SEH frame, scene. Calls: Scene_CreateObject_Gear, __strnicmp, operator_new, Gear_ctor, AthenaList_Append, CreatePlatformOrStands. Offsets: 8, Lines: 59
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
Scene_CreateObject_Gear
          (void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 in_stack_ffffffc4;
  undefined4 in_stack_ffffffc8;
  undefined4 in_stack_ffffffcc;
  undefined4 uVar4;
  char *pcVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9d1b;
  local_c = ExceptionList;
  pvVar2 = (void *)0x0;
  uVar3 = 0;
  uVar4 = 0x418791;
  ExceptionList = &local_c;
  pcVar5 = param_1;
  iVar1 = __strnicmp(param_1,"GEAR",4);
  if (iVar1 == 0) {
    uVar3 = 0x4187aa;
    pvVar2 = operator_new(0x1514);
    local_4 = 0;
    if (pvVar2 == (void *)0x0) {
      pvVar2 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 0x10)) {
        uVar4 = *(undefined4 *)(param_4 + 0x10);
        pcVar5 = *(char **)(param_4 + 0x14);
        uVar3 = *(undefined4 *)(param_4 + 0x18);
      }
      if ((undefined4 *)&stack0xffffffc4 != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffc4 = *(undefined4 *)(param_4 + 4);
        in_stack_ffffffc8 = *(undefined4 *)(param_4 + 8);
        in_stack_ffffffcc = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar2 = Gear_ctor(pvVar2,this,in_stack_ffffffc4,in_stack_ffffffc8,in_stack_ffffffcc,uVar4,
                         pcVar5,uVar3,*(void **)((int)this + 0x47e0));
    }
    uVar3 = *(undefined4 *)((int)pvVar2 + 0x10d4);
    local_4 = 0xffffffff;
    *(undefined4 *)((int)pvVar2 + 0x10f4) = 0x40000000;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar2);
  }
  *param_2 = pvVar2;
  *param_3 = uVar3;
  if (pvVar2 == (void *)0x0) {
    CreatePlatformOrStands(this,param_1,param_2,param_3,param_4);
  }
  ExceptionList = local_c;
  return;
}
