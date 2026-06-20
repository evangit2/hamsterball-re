
void __thiscall UIList_AddSpacer(void *this,int param_1)

{
  void *pvVar1;
  undefined4 *puVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc47b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  pvVar1 = operator_new(0x444);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    puVar2 = (undefined4 *)0x0;
  }
  else {
    puVar2 = (undefined4 *)UIListItem_ctor((int)pvVar1);
  }
  local_4 = 0xffffffff;
  *(undefined1 *)((int)puVar2 + 0x441) = 0;
  *puVar2 = 0;
  *(undefined1 *)(puVar2 + 0x110) = 0;
  puVar2[1] = 0;
  puVar2[9] = param_1;
  puVar2[7] = 0;
  AthenaList_Append((void *)((int)this + 0x88c),(int)puVar2);
  *(int *)((int)this + 0xcb4) = *(int *)((int)this + 0xcb4) + param_1;
  *(undefined1 *)((int)this + 0xcbc) = 1;
  ExceptionList = local_c;
  return;
}

