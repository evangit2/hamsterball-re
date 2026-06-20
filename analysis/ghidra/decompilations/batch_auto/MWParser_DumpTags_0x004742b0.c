
undefined4 * __thiscall MWParser_DumpTags(void *this,undefined4 *param_1)

{
  char *_Memory;
  undefined4 *puVar1;
  char local_48;
  undefined1 local_47;
  undefined1 local_46;
  undefined4 local_44;
  undefined4 local_40 [6];
  undefined **local_28;
  char *local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18;
  undefined4 local_14;
  undefined1 local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdb17;
  pvStack_c = ExceptionList;
  local_44 = 0;
  local_28 = &PTR_AthenaCString_DeletingDtor_004d290c;
  local_24 = (char *)0x0;
  local_1c = 0;
  local_10 = 1;
  local_20 = 0;
  local_14 = 0;
  local_18 = 0;
  ExceptionList = &pvStack_c;
  AthenaString_AssignCStr(&local_28,(char *)0x0);
  local_4 = 1;
  Sprite_DrawColoredRect(local_40);
  local_4 = CONCAT31(local_4._1_3_,2);
  AthenaString_Set(local_40,*(char **)((int)this + 4));
  puVar1 = MWParser_ReadTag((int)local_40);
  _Memory = local_24;
  while (local_24 = _Memory, puVar1 != (undefined4 *)0x0) {
    AthenaString_Assign(&local_28,(char *)puVar1[1]);
    AthenaString_Assign(&local_28,": ");
    AthenaString_Assign(&local_28,(char *)puVar1[2]);
    local_48 = '\r';
    local_47 = 10;
    local_46 = 0;
    AthenaString_Assign(&local_28,&local_48);
    (**(code **)*puVar1)(1);
    puVar1 = MWParser_ReadTag((int)local_40);
    _Memory = local_24;
  }
  *param_1 = &PTR_AthenaCString_DeletingDtor_004d290c;
  param_1[1] = 0;
  param_1[3] = 0;
  *(undefined1 *)(param_1 + 6) = 1;
  param_1[2] = 0;
  param_1[5] = 0;
  *(undefined1 *)(param_1 + 4) = 0;
  AthenaString_AssignCStr(param_1,_Memory);
  *(undefined1 *)(param_1 + 6) = local_10;
  local_44 = 1;
  local_4 = CONCAT31(local_4._1_3_,1);
  StreamReader_dtor(local_40);
  _free(_Memory);
  ExceptionList = pvStack_c;
  return param_1;
}

