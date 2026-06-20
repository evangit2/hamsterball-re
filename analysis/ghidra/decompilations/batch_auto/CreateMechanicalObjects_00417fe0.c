/*
 * Function: CreateMechanicalObjects
 * Address: 0x00417fe0
 * Signature: void __thiscall
CreateMechanicalObjects
          (void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
 *
 * Patterns: allocates, SEH frame. Calls: CreateMechanicalObjects, __strnicmp, operator_new, Looper_ctor, AthenaList_Append, Gear_ctor, strstr, Rotator_ctor. Offsets: 14, Lines: 164
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
CreateMechanicalObjects
          (void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  char *pcVar2;
  undefined4 uVar3;
  void *pvVar4;
  undefined4 in_stack_ffffffc4;
  undefined4 in_stack_ffffffc8;
  void *in_stack_ffffffcc;
  undefined4 uVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9cf7;
  local_c = ExceptionList;
  pvVar4 = (void *)0x0;
  uVar3 = 0;
  uVar5 = 0x418011;
  ExceptionList = &local_c;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"LOOPER",6);
  if (iVar1 == 0) {
    uVar3 = 0x418026;
    pvVar4 = operator_new(0x1500);
    local_4 = 0;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar5 = *(undefined4 *)(param_4 + 4);
        pcVar2 = *(char **)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      in_stack_ffffffc8 = 0x418064;
      in_stack_ffffffcc = this;
      pvVar4 = Looper_ctor(pvVar4,this,uVar5,pcVar2,uVar3,*(void **)((int)this + 0x436c));
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10d4);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  uVar5 = 0x418095;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"GEAR",4);
  if (iVar1 == 0) {
    uVar3 = 0x4180aa;
    pvVar4 = operator_new(0x1514);
    local_4 = 1;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 0x10)) {
        uVar5 = *(undefined4 *)(param_4 + 0x10);
        pcVar2 = *(char **)(param_4 + 0x14);
        uVar3 = *(undefined4 *)(param_4 + 0x18);
      }
      if ((undefined4 *)&stack0xffffffc4 != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffc4 = *(undefined4 *)(param_4 + 4);
        in_stack_ffffffc8 = *(undefined4 *)(param_4 + 8);
        in_stack_ffffffcc = *(void **)(param_4 + 0xc);
      }
      pvVar4 = Gear_ctor(pvVar4,this,in_stack_ffffffc4,in_stack_ffffffc8,in_stack_ffffffcc,uVar5,
                         pcVar2,uVar3,*(void **)((int)this + 0x4370));
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10d4);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  uVar5 = 0x41813d;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"BIGGEAR",7);
  if (iVar1 == 0) {
    uVar3 = 0x418152;
    pvVar4 = operator_new(0x1514);
    local_4 = 2;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 0x10)) {
        uVar5 = *(undefined4 *)(param_4 + 0x10);
        pcVar2 = *(char **)(param_4 + 0x14);
        uVar3 = *(undefined4 *)(param_4 + 0x18);
      }
      if ((undefined4 *)&stack0xffffffc4 != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffc4 = *(undefined4 *)(param_4 + 4);
        in_stack_ffffffc8 = *(undefined4 *)(param_4 + 8);
        in_stack_ffffffcc = *(void **)(param_4 + 0xc);
      }
      pvVar4 = Gear_ctor(pvVar4,this,in_stack_ffffffc4,in_stack_ffffffc8,in_stack_ffffffcc,uVar5,
                         pcVar2,uVar3,*(void **)((int)this + 0x4374));
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10d4);
    local_4 = 0xffffffff;
    *(undefined4 *)((int)pvVar4 + 0x10f4) = 0x3f000000;
    pcVar2 = strstr(param_1,"TOUCH");
    if (pcVar2 != (char *)0x0) {
      *(undefined1 *)((int)pvVar4 + 0x1510) = 1;
    }
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  uVar5 = 0x41820c;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"ROTATOR",7);
  if (iVar1 == 0) {
    uVar3 = 0x418221;
    pvVar4 = operator_new(0x1508);
    local_4 = 3;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar5 = *(undefined4 *)(param_4 + 4);
        pcVar2 = *(char **)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Rotator_ctor(pvVar4,this,uVar5,pcVar2,uVar3,*(void **)((int)this + 0x4378));
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10d4);
    local_4 = 0xffffffff;
    *(undefined4 *)((int)pvVar4 + 0x10e8) = 0x3f800000;
    iVar1 = RNG_Rand(&PTR_OBJ_VTABLE,2,'\0');
    if (iVar1 == 0) {
      *(undefined4 *)((int)pvVar4 + 0x10e8) = 0xbf800000;
    }
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  uVar5 = 0x4182ba;
  iVar1 = __strnicmp(param_1,"PENDULUM",8);
  if (iVar1 == 0) {
    uVar3 = 0x4182cb;
    pvVar4 = operator_new(0x1504);
    local_4 = 4;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar5 = *(undefined4 *)(param_4 + 4);
        param_1 = *(char **)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Pendulum_ctor(pvVar4,this,uVar5,param_1,uVar3,*(void **)((int)this + 0x437c));
    }
    uVar5 = *(undefined4 *)((int)pvVar4 + 0x10d4);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
    *param_2 = pvVar4;
    *param_3 = uVar5;
  }
  else {
    *param_2 = pvVar4;
    *param_3 = uVar3;
  }
  ExceptionList = local_c;
  return;
}
