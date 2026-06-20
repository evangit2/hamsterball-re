
void String_AllocBuffer(uint param_1)

{
  undefined4 *puVar1;
  undefined4 extraout_ECX;
  uint uVar2;
  void *pvStack_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_004cad60;
  pvStack_10 = ExceptionList;
  uVar2 = param_1 | 0xf;
  if ((param_1 | 0xf) == 0xffffffff) {
    uVar2 = param_1;
  }
  local_8 = 0;
  ExceptionList = &pvStack_10;
  puVar1 = operator_new(uVar2 + 1);
  AthenaString_MoveAssign(extraout_ECX,puVar1);
  return;
}

