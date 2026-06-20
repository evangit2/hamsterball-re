
void __thiscall D3DXSkinMesh_CreateStrip(void *this,int param_1,int param_2)

{
  void *this_00;
  undefined4 *puVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdf4b;
  local_c = ExceptionList;
  this_00 = (void *)((int)this + 0x450);
  ExceptionList = &local_c;
  AthenaList_Free((int)this_00);
  AthenaList_Append(this_00,param_1);
  AthenaList_Append(this_00,param_2);
  puVar1 = operator_new(0x14);
  local_4 = 0;
  if (puVar1 == (undefined4 *)0x0) {
    iVar2 = 0;
  }
  else {
    iVar2 = Scene_CtorBase(puVar1);
  }
  *(int *)((int)this + 0x44c) = iVar2;
  *(undefined4 *)(iVar2 + 4) = 0;
  ExceptionList = local_c;
  return;
}

