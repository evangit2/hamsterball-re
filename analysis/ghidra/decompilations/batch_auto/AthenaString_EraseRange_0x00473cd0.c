
void __thiscall AthenaString_EraseRange(void *this,size_t param_1,int param_2)

{
  char cVar1;
  char *pcVar2;
  undefined **ppuVar3;
  char *pcVar4;
  undefined **local_44;
  char *local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 local_34;
  undefined4 local_30;
  undefined1 local_2c;
  undefined **local_28;
  char *local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18;
  undefined4 local_14;
  undefined1 local_10;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdaa0;
  local_c = ExceptionList;
  if (-1 < (int)param_1) {
    local_44 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_40 = (char *)0x0;
    local_38 = 0;
    local_2c = 1;
    local_3c = 0;
    local_30 = 0;
    local_34 = 0;
    ExceptionList = &local_c;
    AthenaString_AssignCStr(&local_44,(char *)0x0);
    local_4 = 0;
    local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_24 = (char *)0x0;
    local_1c = 0;
    local_10 = 1;
    local_20 = 0;
    local_14 = 0;
    local_18 = 0;
    AthenaString_AssignCStr(&local_28,*(char **)((int)this + 4));
    pcVar2 = local_24;
    local_10 = *(undefined1 *)((int)this + 0x18);
    local_4._0_1_ = 1;
    StdString_AppendN(&local_44,local_24,param_1);
    local_4 = (uint)local_4._1_3_ << 8;
    _free(pcVar2);
    if (*(char *)((int)this + 0x10) != '\0') {
      pcVar2 = *(char **)((int)this + 4);
      if (pcVar2 == (char *)0x0) {
        *(undefined4 *)((int)this + 0x14) = 0;
      }
      else {
        pcVar4 = pcVar2 + 1;
        do {
          cVar1 = *pcVar2;
          pcVar2 = pcVar2 + 1;
        } while (cVar1 != '\0');
        *(int *)((int)this + 0x14) = (int)pcVar2 - (int)pcVar4;
      }
      *(undefined1 *)((int)this + 0x10) = 0;
    }
    pcVar2 = local_40;
    if ((int)(param_1 + param_2) <= *(int *)((int)this + 0x14)) {
      AthenaString_Assign(&local_44,(char *)(*(int *)((int)this + 4) + param_1 + param_2));
      pcVar2 = local_40;
      pcVar4 = (char *)0x0;
      ppuVar3 = &PTR_AthenaCString_DeletingDtor_004d290c;
      AthenaString_AssignCStr(&stack0xffffff90,local_40);
      AthenaString_AssignCStrFree2(this,ppuVar3,pcVar4);
    }
    _free(pcVar2);
  }
  ExceptionList = local_c;
  return;
}

