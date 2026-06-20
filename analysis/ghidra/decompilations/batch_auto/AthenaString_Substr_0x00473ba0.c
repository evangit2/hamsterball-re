
undefined4 * __thiscall
AthenaString_Substr(void *this,undefined4 *param_1,int param_2,size_t param_3)

{
  char *pcVar1;
  char cVar2;
  char *pcVar3;
  undefined **local_28;
  char *local_24;
  uint local_20;
  undefined4 local_1c;
  undefined1 local_18;
  undefined4 local_14;
  undefined1 local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cda48;
  local_c = ExceptionList;
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = (char *)0x0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  ExceptionList = &local_c;
  AthenaString_AssignCStr(&local_28,(char *)0x0);
  local_4 = 0;
  if (*(char *)((int)this + 0x10) != '\0') {
    pcVar3 = *(char **)((int)this + 4);
    if (pcVar3 == (char *)0x0) {
      *(undefined4 *)((int)this + 0x14) = 0;
    }
    else {
      pcVar1 = pcVar3 + 1;
      do {
        cVar2 = *pcVar3;
        pcVar3 = pcVar3 + 1;
      } while (cVar2 != '\0');
      *(int *)((int)this + 0x14) = (int)pcVar3 - (int)pcVar1;
    }
    *(undefined1 *)((int)this + 0x10) = 0;
  }
  pcVar3 = local_24;
  if (param_2 < *(int *)((int)this + 0x14)) {
    _free(local_24);
    local_24 = (char *)0x0;
    local_14 = 0;
    local_20 = 0;
    if (0 < (int)(param_3 + 1)) {
      local_20 = param_3 + 1;
    }
    pcVar3 = operator_new(local_20);
    local_24 = pcVar3;
    _strncpy(pcVar3,(char *)(*(int *)((int)this + 4) + param_2),param_3);
    pcVar3[param_3] = '\0';
    local_10 = 0;
  }
  *param_1 = &PTR_AthenaCString_DeletingDtor_004d290c;
  param_1[1] = 0;
  param_1[3] = 0;
  *(undefined1 *)(param_1 + 6) = 1;
  param_1[2] = 0;
  param_1[5] = 0;
  *(undefined1 *)(param_1 + 4) = 0;
  AthenaString_AssignCStr(param_1,pcVar3);
  *(undefined1 *)(param_1 + 6) = local_10;
  _free(pcVar3);
  ExceptionList = local_c;
  return param_1;
}

