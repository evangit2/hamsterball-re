
void __fastcall TourneyMenu_Advance(int param_1)

{
  int iVar1;
  void *this;
  int *piVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cac9b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined1 *)(*(int *)(*(int *)(param_1 + 4) + 0x220) + 0x95) = 1;
  iVar1 = *(int *)(*(int *)(param_1 + 4) + 0x220);
  *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + -1;
  this = operator_new(0x111c);
  local_4 = 0;
  if (this == (void *)0x0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = TourneyMenu_ctor(this,*(int *)(param_1 + 4),'\0');
  }
  local_4 = 0xffffffff;
  Scene_AddObject(*(void **)(*(int *)(param_1 + 4) + 0x184),piVar2);
  ExceptionList = local_c;
  return;
}

