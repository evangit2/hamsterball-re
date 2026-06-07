// CreateSawblade (0x40E250) — Raw Ghidra decompilation
// Multi-factory: handles BONK, TIP (tower), SAWBLADE, BRIDGE (spinner), JUDGE (gear), BELL
// THIS IS NOT JUST SAWBLADE — it's the arena object sub-dispatcher

void __thiscall
CreateSawblade(void *this,char *param_1,undefined4 *param_2,undefined4 *param_3,int param_4)
{
  int iVar1;
  void *pvVar2;
  char *pcVar3;
  void *pvVar4;
  float fVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 local_14;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9782;
  local_c = ExceptionList;
  pvVar4 = (void *)0x0;
  local_14 = 0;
  ExceptionList = &local_c;
  
  // BONK
  pcVar3 = param_1;
  iVar1 = __strnicmp(param_1,"BONK",4);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    uVar7 = 0x1200;
    uVar6 = 0x40e2a4;
    pvVar2 = operator_new(0x1200);
    local_4 = 0;
    if (pvVar2 != (void *)0x0) {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        pcVar3 = *(char **)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        uVar7 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Bonk_ctor(pvVar2,(int)this,(float)pcVar3,uVar6,uVar7);
    }
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
    local_14 = *(undefined4 *)((int)pvVar4 + 0x10f8);
    *(void **)((int)this + 0x436c) = pvVar4;
  }
  
  // TIP (TowerLevel)
  fVar5 = 5.958929e-39;
  pcVar3 = param_1;
  iVar1 = __strnicmp(param_1,(char *)&PTR_DAT_004cfa48,3);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    uVar6 = 0x40e340;
    pvVar4 = operator_new(0x1188);
    local_4 = 1;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((float *)&stack0xffffffcc != (float *)(param_4 + 4)) {
        fVar5 = *(float *)(param_4 + 4);
        pcVar3 = *(char **)(param_4 + 8);
        uVar6 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = TowerLevel_Ctor(pvVar4,(int)this,fVar5,pcVar3,uVar6,*(float *)(param_4 + 0x14));
    }
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
    pcVar3 = strstr(param_1,"SLOW");
    if (pcVar3 != (char *)0x0) {
      *(undefined1 *)((int)pvVar4 + 0x10ec) = 1;
    }
    pcVar3 = strstr(param_1,"SUPER");
    if (pcVar3 != (char *)0x0) {
      *(undefined1 *)((int)pvVar4 + 0x10ed) = 1;
    }
    pcVar3 = strstr(param_1,"UP");
    if (pcVar3 != (char *)0x0) {
      Sound_InitChannels(pvVar4,'\x01');
    }
  }
  
  // SAWBLADE
  pcVar3 = param_1;
  iVar1 = __strnicmp(param_1,"SAWBLADE",8);
  if ((iVar1 == 0) && (*(int *)(*(int *)((int)this + 0x878) + 0x23c) != 0)) {
    uVar7 = 0x111c;
    uVar6 = 0x40e419;
    pvVar4 = operator_new(0x111c);
    local_4 = 2;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        pcVar3 = *(char **)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        uVar7 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Sawblade_Level_Ctor(pvVar4,(int)this,pcVar3,uVar6,uVar7);
    }
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
    pcVar3 = strstr(param_1,"1");
    if (pcVar3 != (char *)0x0) {
      *(void **)((int)this + 0x4370) = pvVar4;
      Sawblade_SetBreakSound(pvVar4,1);
    }
    pcVar3 = strstr(param_1,"2");
    if (pcVar3 != (char *)0x0) {
      *(void **)((int)this + 0x4374) = pvVar4;
      Sawblade_SetBreakSound(pvVar4,2);
    }
  }
  
  // BRIDGE (Spinner)
  pvVar2 = (void *)0x40e4c0;
  pcVar3 = param_1;
  iVar1 = __strnicmp(param_1,"BRIDGE",6);
  if (iVar1 == 0) {
    uVar6 = 0x40e4d5;
    pvVar4 = operator_new(0x10fc);
    local_4 = 3;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffcc != (undefined4 *)(param_4 + 4)) {
        pvVar2 = *(void **)(param_4 + 4);
        pcVar3 = *(char **)(param_4 + 8);
        uVar6 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Spinner_Level_ctor(pvVar4,(int)this,pvVar2,pcVar3,uVar6,*(float *)(param_4 + 0x14));
    }
    local_14 = *(undefined4 *)((int)pvVar4 + 0x10f4);
    local_4 = 0xffffffff;
    pcVar3 = strstr(param_1,"1");
    if (pcVar3 != (char *)0x0) {
      AthenaList_Append((void *)((int)this + 0x4380),(int)pvVar4);
    }
    pcVar3 = strstr(param_1,"2");
    if (pcVar3 != (char *)0x0) {
      AthenaList_Append((void *)((int)this + 0x4798),(int)pvVar4);
    }
    pcVar3 = strstr(param_1,"NEG");
    if (pcVar3 != (char *)0x0) {
      *(undefined4 *)((int)pvVar4 + 0x10f8) = 0xbf800000;
    }
  }
  
  // JUDGE (Gear)
  pcVar3 = param_1;
  iVar1 = __strnicmp(param_1,"JUDGE",5);
  if (iVar1 == 0) {
    uVar7 = 0x1100;
    uVar6 = 0x40e5ac;
    pvVar4 = operator_new(0x1100);
    local_4 = 4;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        pcVar3 = *(char **)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        uVar7 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Gear_Level_ctor(pvVar4,(int)this,pcVar3,uVar6,uVar7);
    }
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 0x4bbc),(int)pvVar4);
  }
  
  // BELL (Tipper variant)
  iVar1 = __strnicmp(param_1,"BELL",4);
  if (iVar1 == 0) {
    uVar7 = 0x10e8;
    uVar6 = 0x40e61f;
    pvVar4 = operator_new(0x10e8);
    local_4 = 5;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      if ((undefined4 *)&stack0xffffffd0 != (undefined4 *)(param_4 + 4)) {
        param_1 = *(char **)(param_4 + 4);
        uVar6 = *(undefined4 *)(param_4 + 8);
        uVar7 = *(undefined4 *)(param_4 + 0xc);
      }
      pvVar4 = Tipper_Level_Ctor(pvVar4,(int)this,param_1,uVar6,uVar7);
    }
    local_4 = 0xffffffff;
    *(void **)((int)this + 0x4fd4) = pvVar4;
    AthenaList_Append((void *)((int)this + 0x2578),(int)pvVar4);
  }
  *param_2 = pvVar4;
  *param_3 = local_14;
  ExceptionList = local_c;
  return;
}