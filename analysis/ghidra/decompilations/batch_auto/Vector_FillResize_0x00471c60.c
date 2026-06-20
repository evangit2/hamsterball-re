
void __thiscall Vector_FillResize(void *this,int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_004cd900;
  local_10 = ExceptionList;
  puVar1 = *(undefined4 **)((int)this + 4);
  local_8 = 0;
  ExceptionList = &local_10;
  Array_FillDWords(puVar1,param_1,param_2);
  *(undefined4 **)((int)this + 8) = puVar1 + param_1;
  ExceptionList = local_10;
  return;
}

