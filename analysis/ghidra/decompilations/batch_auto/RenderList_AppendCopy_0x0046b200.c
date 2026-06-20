
void __thiscall RenderList_AppendCopy(void *this,undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd51b;
  local_c = ExceptionList;
  if (*(char *)((int)this + 0x420) == '\0') {
    ExceptionList = &local_c;
    puVar1 = operator_new(0x50);
    local_4 = 0;
    if (puVar1 == (undefined4 *)0x0) {
      puVar1 = (undefined4 *)0x0;
    }
    else {
      puVar1 = RenderContext_Init(puVar1);
    }
    puVar3 = puVar1;
    for (iVar2 = 0x14; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = *param_1;
      param_1 = param_1 + 1;
      puVar3 = puVar3 + 1;
    }
    local_4 = 0xffffffff;
    AthenaList_Append((void *)((int)this + 4),(int)puVar1);
    ExceptionList = local_c;
    return;
  }
  ExceptionList = &local_c;
  AthenaList_Append((void *)((int)this + 4),(int)param_1);
  ExceptionList = local_c;
  return;
}

