/*
 * Function: CreateSpeedCylinder
 * Address: 0x004117b0
 * Signature: void __thiscall
CreateSpeedCylinder(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
 *
 * Patterns: allocates, SEH frame. Calls: CreateSpeedCylinder, __strnicmp, _atol, operator_new, Rotator_ctor_sound, AthenaList_Append, __ftol2, Pendulum_ctor. Offsets: 6, Lines: 98
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
CreateSpeedCylinder(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  long lVar2;
  undefined4 uVar3;
  void *pvVar4;
  ulonglong uVar5;
  undefined4 in_stack_ffffffcc;
  undefined4 uVar6;
  char *pcVar7;
  void *pvVar8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c99a1;
  local_c = ExceptionList;
  pvVar4 = (void *)0x0;
  uVar3 = 0;
  uVar6 = 0x4117e1;
  ExceptionList = &local_c;
  iVar1 = __strnicmp(param_1,"LIFTER",6);
  if (iVar1 == 0) {
    lVar2 = _atol(param_1 + 6);
    uVar3 = 0x411805;
    pvVar4 = operator_new(0x10f4);
    local_4 = 0;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffcc != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffcc = *(undefined4 *)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Rotator_ctor_sound(pvVar4,this,in_stack_ffffffcc,uVar6,uVar3,
                                  *(void **)((int)this + 0x4784),lVar2);
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  uVar6 = 0x411875;
  pcVar7 = param_1;
  iVar1 = __strnicmp(param_1,"SPEEDCYLINDER",0xd);
  if (iVar1 == 0) {
    pvVar4 = operator_new(0x150c);
    local_4 = 1;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      pvVar8 = *(void **)((int)this + 0x4788);
      uVar5 = __ftol2();
      if ((undefined4 *)&stack0xffffffcc != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffcc = *(undefined4 *)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        pcVar7 = *(char **)(param_4 + 0xc);
      }
      pvVar4 = Pendulum_ctor(pvVar4,this,in_stack_ffffffcc,uVar6,pcVar7,(int)uVar5,pvVar8);
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  uVar6 = 0x41190a;
  iVar1 = __strnicmp(param_1,"TIMEBUTTON",10);
  if (iVar1 == 0) {
    uVar3 = 0x41191b;
    pvVar4 = operator_new(0x10e8);
    local_4 = 2;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar6 = *(undefined4 *)(param_4 + 4);
        param_1 = *(char **)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Rotator_ctor_nosound(pvVar4,this,uVar6,param_1,uVar3,*(void **)((int)this + 0x478c));
    }
    uVar6 = *(undefined4 *)((int)pvVar4 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
    *param_2 = pvVar4;
    *param_3 = uVar6;
  }
  else {
    *param_2 = pvVar4;
    *param_3 = uVar3;
  }
  ExceptionList = local_c;
  return;
}
