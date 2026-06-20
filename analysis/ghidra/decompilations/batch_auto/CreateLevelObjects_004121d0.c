/*
 * Function: CreateLevelObjects
 * Address: 0x004121d0
 * Signature: CreateLevelObjects(...)
 *
 * Patterns: allocates, SEH frame, scene, level. Calls: CreateLevelObjects, __strnicmp, strstr, operator_new, Tipper_ctor, TipperVisual_ctor, TipperVisual_Attach, AthenaList_Append. Offsets: 38, Lines: 267
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* CreateLevelObjects: Factory dispatcher using strnicmp on mesh names. Creates
   BRIDGE/TIPPER/BONK/BBRIDGE1-2/POPCYLINDER/BLOCKDAWG1-2/CATAPULT/GLUEBIE. All added to
   Scene+0x2578. See decompilations/scene/decomp_object_factory.c */

void __thiscall
CreateLevelObjects(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  char *pcVar2;
  void *pvVar3;
  undefined4 uVar4;
  void *pvVar5;
  void *in_stack_ffffffcc;
  undefined4 uVar6;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9a5e;
  local_c = ExceptionList;
  pvVar5 = (void *)0x0;
  uVar4 = 0;
  ExceptionList = &local_c;
  iVar1 = __strnicmp(param_1,"BRIDGE",6);
  if (iVar1 == 0) {
    pvVar5 = *(void **)((int)this + 0x436c);
    if ((undefined4 *)((int)this + 0x437c) != (undefined4 *)(param_4 + 4)) {
      *(undefined4 *)((int)this + 0x437c) = *(undefined4 *)(param_4 + 4);
      *(undefined4 *)((int)this + 0x4380) = *(undefined4 *)(param_4 + 8);
      *(undefined4 *)((int)this + 0x4384) = *(undefined4 *)(param_4 + 0xc);
    }
    pcVar2 = strstr(param_1,"(NOCOLLIDE)");
    if (pcVar2 == (char *)0x0) {
      uVar4 = *(undefined4 *)((int)this + 0x4370);
    }
  }
  iVar1 = __strnicmp(param_1,"TIPPER",6);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    pvVar3 = operator_new(0x1104);
    pvVar5 = (void *)0x0;
    local_4 = 0;
    if (pvVar3 != (void *)0x0) {
      pvVar5 = Tipper_ctor(pvVar3,this,*(int *)((int)this + 0x4394));
    }
    local_4 = 0xffffffff;
    if ((undefined4 *)((int)pvVar5 + 0x10d8) != (undefined4 *)(param_4 + 4)) {
      *(undefined4 *)((int)pvVar5 + 0x10d8) = *(undefined4 *)(param_4 + 4);
      *(undefined4 *)((int)pvVar5 + 0x10dc) = *(undefined4 *)(param_4 + 8);
      *(undefined4 *)((int)pvVar5 + 0x10e0) = *(undefined4 *)(param_4 + 0xc);
    }
    if ((undefined4 *)((int)pvVar5 + 0x10e4) != (undefined4 *)(param_4 + 0x10)) {
      *(undefined4 *)((int)pvVar5 + 0x10e4) = *(undefined4 *)(param_4 + 0x10);
      *(undefined4 *)((int)pvVar5 + 0x10e8) = *(undefined4 *)(param_4 + 0x14);
      *(undefined4 *)((int)pvVar5 + 0x10ec) = *(undefined4 *)(param_4 + 0x18);
    }
    pvVar3 = operator_new(0x10d0);
    local_4 = 1;
    if (pvVar3 == (void *)0x0) {
      pvVar3 = (void *)0x0;
    }
    else {
      pvVar3 = TipperVisual_ctor(pvVar3,*(int *)((int)this + 0x4398));
    }
    local_4 = 0xffffffff;
    *(void **)((int)pvVar5 + 0x10d4) = pvVar3;
    TipperVisual_Attach(pvVar3,(int)pvVar5);
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
  }
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"BONK",4);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    uVar6 = 0x1200;
    uVar4 = 0x41236c;
    pvVar5 = operator_new(0x1200);
    local_4 = 2;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd4 != (undefined4 *)(param_4 + 4)) {
        pcVar2 = *(char **)(param_4 + 4);
        uVar4 = *(undefined4 *)(param_4 + 8);
        uVar6 = *(undefined4 *)(param_4 + 0xc);
      }
      in_stack_ffffffcc = (void *)0x4123a7;
      pvVar5 = Bonk_ctor(pvVar5,(int)this,(float)pcVar2,uVar4,uVar6);
    }
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10f8);
    *(void **)((int)this + 0x540c) = pvVar5;
  }
  uVar6 = 0x4123de;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"BBRIDGE1",8);
  if (iVar1 == 0) {
    uVar4 = 0x4123ef;
    pvVar5 = operator_new(0x1100);
    local_4 = 3;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar6 = *(undefined4 *)(param_4 + 4);
        pcVar2 = *(char **)(param_4 + 8);
        uVar4 = *(undefined4 *)(param_4 + 0xc);
      }
      in_stack_ffffffcc = this;
      pvVar5 = BreakBridge_ctor(pvVar5,this,uVar6,pcVar2,uVar4,*(void **)((int)this + 0x5410));
    }
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
    *(void **)((int)this + 0x5418) = pvVar5;
  }
  uVar6 = 0x412468;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"BBRIDGE2",8);
  if (iVar1 == 0) {
    uVar4 = 0x412479;
    pvVar5 = operator_new(0x1100);
    local_4 = 4;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar6 = *(undefined4 *)(param_4 + 4);
        pcVar2 = *(char **)(param_4 + 8);
        uVar4 = *(undefined4 *)(param_4 + 0xc);
      }
      in_stack_ffffffcc = this;
      pvVar5 = BreakBridge_ctor(pvVar5,this,uVar6,pcVar2,uVar4,*(void **)((int)this + 0x5414));
    }
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
    *(void **)((int)this + 0x541c) = pvVar5;
  }
  uVar6 = 0x4124f2;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"POPCYLINDER",0xb);
  if (iVar1 == 0) {
    uVar4 = 0x412503;
    pvVar5 = operator_new(0x10e8);
    local_4 = 5;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        uVar6 = *(undefined4 *)(param_4 + 4);
        pcVar2 = *(char **)(param_4 + 8);
        uVar4 = *(undefined4 *)(param_4 + 0xc);
      }
      in_stack_ffffffcc = this;
      pvVar5 = PopCylinder_ctor(pvVar5,this,uVar6,pcVar2,uVar4,*(void **)((int)this + 0x5420));
    }
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10e0);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
    AthenaList_Append((void *)((int)this + 0x5428),(int)pvVar5);
  }
  uVar6 = 0x412582;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"BLOCKDAWG1",10);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    pvVar5 = operator_new(0x1154);
    local_4 = 6;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      iVar1 = Level_FindObjectByName(*(void **)((int)this + 0x8ac),"DAWGPATH1");
      if ((undefined4 *)&stack0xffffffcc != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffcc = *(void **)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        pcVar2 = *(char **)(param_4 + 0xc);
      }
      pvVar5 = Blockdawg_ctor(pvVar5,this,(int)in_stack_ffffffcc,uVar6,pcVar2,
                              *(void **)((int)this + 0x5840),iVar1);
    }
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10d4);
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
  }
  uVar6 = 0x41262d;
  pcVar2 = param_1;
  iVar1 = __strnicmp(param_1,"BLOCKDAWG2",10);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    pvVar5 = operator_new(0x1154);
    local_4 = 7;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      iVar1 = Level_FindObjectByName(*(void **)((int)this + 0x8ac),"DAWGPATH2");
      if ((undefined4 *)&stack0xffffffcc != (undefined4 *)(param_4 + 4)) {
        in_stack_ffffffcc = *(void **)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        pcVar2 = *(char **)(param_4 + 0xc);
      }
      pvVar5 = Blockdawg_ctor(pvVar5,this,(int)in_stack_ffffffcc,uVar6,pcVar2,
                              *(void **)((int)this + 0x5844),iVar1);
    }
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10d4);
    local_4 = 0xffffffff;
    *(undefined1 *)((int)pvVar5 + 0x1152) = 1;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
  }
  iVar1 = __strnicmp(param_1,"CATAPULT",8);
  if (iVar1 == 0) {
    pvVar5 = operator_new(0x1108);
    local_4 = 8;
    if (pvVar5 == (void *)0x0) {
      pvVar5 = (void *)0x0;
    }
    else {
      pvVar5 = Catapult_ctor(pvVar5,this,*(int *)((int)this + 0x5848));
    }
    local_4 = 0xffffffff;
    *(undefined1 *)((int)pvVar5 + 0x1100) = 1;
    if ((undefined4 *)((int)pvVar5 + 0x10d8) != (undefined4 *)(param_4 + 4)) {
      *(undefined4 *)((int)pvVar5 + 0x10d8) = *(undefined4 *)(param_4 + 4);
      *(undefined4 *)((int)pvVar5 + 0x10dc) = *(undefined4 *)(param_4 + 8);
      *(undefined4 *)((int)pvVar5 + 0x10e0) = *(undefined4 *)(param_4 + 0xc);
    }
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
    AthenaList_Append((void *)((int)this + 0x584c),(int)pvVar5);
    uVar4 = *(undefined4 *)((int)pvVar5 + 0x10d4);
  }
  iVar1 = __strnicmp(param_1,"GLUEBIE",7);
  if (iVar1 == 0) {
    if (*(int *)(*(int *)((int)this + 0x878) + 0x23c) == 0) {
      *param_2 = pvVar5;
      *param_3 = uVar4;
    }
    else {
      pvVar5 = operator_new(0x110c);
      local_4 = 9;
      if (pvVar5 == (void *)0x0) {
        pvVar5 = (void *)0x0;
      }
      else {
        pvVar5 = Gluebie_ctor(pvVar5,this,*(int *)((int)this + 0x607c));
      }
      local_4 = 0xffffffff;
      if ((undefined4 *)((int)pvVar5 + 0x10d4) != (undefined4 *)(param_4 + 4)) {
        *(undefined4 *)((int)pvVar5 + 0x10d4) = *(undefined4 *)(param_4 + 4);
        *(undefined4 *)((int)pvVar5 + 0x10d8) = *(undefined4 *)(param_4 + 8);
        *(undefined4 *)((int)pvVar5 + 0x10dc) = *(undefined4 *)(param_4 + 0xc);
      }
      AthenaList_Append((void *)((int)this + 0x6080),(int)pvVar5);
      AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar5);
      *param_2 = pvVar5;
      *param_3 = uVar4;
    }
  }
  else {
    *param_2 = pvVar5;
    *param_3 = uVar4;
  }
  ExceptionList = local_c;
  return;
}
