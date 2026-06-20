/*
 * Function: CreateLifter
 * Address: 0x00414a20
 * Signature: void __thiscall
CreateLifter(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
 *
 * Patterns: allocates, SEH frame. Calls: CreateLifter, __strnicmp, _atol, operator_new, Rotator_ctor_sound, AthenaList_Append, CreatePlatformOrStands. Offsets: 4, Lines: 50
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
CreateLifter(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  long lVar2;
  undefined4 uVar3;
  void *pvVar4;
  undefined4 in_stack_ffffffcc;
  undefined4 uVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9c6b;
  local_c = ExceptionList;
  pvVar4 = (void *)0x0;
  uVar3 = 0;
  uVar5 = 0x414a51;
  ExceptionList = &local_c;
  iVar1 = __strnicmp(param_1,"LIFTER",6);
  if (iVar1 == 0) {
    lVar2 = _atol(param_1 + 6);
    uVar3 = 0x414a6d;
    pvVar4 = operator_new(0x10f4);
    local_4 = 0;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffcc != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffcc = *(undefined4 *)(param_4 + 4);
        uVar5 = *(undefined4 *)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Rotator_ctor_sound(pvVar4,this,in_stack_ffffffcc,uVar5,uVar3,
                                  *(void **)((int)this + 0x47e0),lVar2);
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  *param_2 = pvVar4;
  *param_3 = uVar3;
  if (pvVar4 == (void *)0x0) {
    CreatePlatformOrStands(this,param_1,param_2,param_3,param_4);
  }
  ExceptionList = local_c;
  return;
}
