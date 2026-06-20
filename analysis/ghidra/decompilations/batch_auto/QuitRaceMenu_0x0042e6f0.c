
void * __thiscall QuitRaceMenu(void *this,int param_1)

{
  undefined4 in_stack_ffffffd4;
  undefined4 in_stack_ffffffd8;
  undefined4 in_stack_ffffffdc;
  undefined4 in_stack_ffffffe0;
  undefined4 uVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb398;
  local_c = ExceptionList;
  uVar1 = 0x42e717;
  ExceptionList = &local_c;
  SimpleMenu_ctor(this,param_1);
  iVar2 = 0;
  local_4 = 0;
  *(undefined ***)this = &PTR_MPMenu_DeletingDtor2_004d41b0;
  *(char **)((int)this + 0x868) = "Pause Menu";
  Matrix_Scale4x4(&stack0xffffffd4,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"RESUME","RESUME",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar1,iVar2);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xffffffd4,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"RESTART RACE","RESTART",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc
                 ,in_stack_ffffffe0,uVar1,iVar2);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  UIList_AddItem(this,"OPTIONS","OP",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar1,iVar2);
  uVar1 = 0x42e7d7;
  UIList_AddSpacer(this,10);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f400000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"QUIT THIS RACE","BACK",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar1,iVar2);
  ExceptionList = local_c;
  return this;
}

