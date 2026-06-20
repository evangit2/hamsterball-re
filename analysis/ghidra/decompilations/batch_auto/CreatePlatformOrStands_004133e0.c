/*
 * Function: CreatePlatformOrStands
 * Address: 0x004133e0
 * Signature: void __thiscall
CreatePlatformOrStands(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
 *
 * Patterns: allocates, vtable dispatch, SEH frame. Calls: CreatePlatformOrStands, __strnicmp, operator_new, Platform_ctor, AthenaList_Append, Stands_ctor, Timer_Init, Timer_Cleanup. Offsets: 8, Lines: 77
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
CreatePlatformOrStands(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  void *this_00;
  int *piVar2;
  undefined4 uVar3;
  void *pvVar4;
  undefined4 uVar5;
  char *pcVar6;
  undefined4 uStack_5c;
  undefined4 uStack_58;
  undefined4 uStack_54;
  int aiStack_50 [17];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9aae;
  local_c = ExceptionList;
  pvVar4 = (void *)0x0;
  uVar3 = 0;
  uVar5 = 0x413413;
  ExceptionList = &local_c;
  pcVar6 = param_1;
  iVar1 = __strnicmp(param_1,"PLATFORM",8);
  if (iVar1 == 0) {
    uVar3 = 0x413424;
    this_00 = operator_new(0x10fc);
    local_4 = 0;
    if (this_00 != (void *)0x0) {
      if ((undefined4 *)&stack0xffffff80 != (undefined4 *)(param_4 + 4)) {
        uVar5 = *(undefined4 *)(param_4 + 4);
        pcVar6 = *(char **)(param_4 + 8);
        uVar3 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Platform_ctor(this_00,this,uVar5,pcVar6,uVar3,*(void **)((int)this + 0x438c));
    }
    uVar3 = *(undefined4 *)((int)pvVar4 + 0x10ec);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
    AthenaList_Append((void *)((int)this + 0x4394),(int)pvVar4);
  }
  iVar1 = __strnicmp(param_1,"STANDS",6);
  if (iVar1 == 0) {
    pvVar4 = operator_new(0x10d0);
    local_4 = 1;
    if (pvVar4 == (void *)0x0) {
      piVar2 = (int *)0x0;
    }
    else {
      piVar2 = Stands_ctor(pvVar4,*(int *)((int)this + 0x4390));
    }
    local_4 = 0xffffffff;
    (**(code **)(*piVar2 + 0x58))();
    Timer_Init(aiStack_50);
    local_4 = 2;
    if (&uStack_5c != (undefined4 *)(param_4 + 4)) {
      uStack_5c = *(undefined4 *)(param_4 + 4);
      uStack_58 = *(undefined4 *)(param_4 + 8);
      uStack_54 = *(undefined4 *)(param_4 + 0xc);
    }
    (**(code **)(aiStack_50[0] + 8))();
    (**(code **)(*piVar2 + 0x54))();
    local_4 = 0xffffffff;
    Timer_Cleanup(aiStack_50);
    *param_2 = piVar2;
    *param_3 = uVar3;
  }
  else {
    *param_2 = pvVar4;
    *param_3 = uVar3;
  }
  ExceptionList = local_c;
  return;
}
