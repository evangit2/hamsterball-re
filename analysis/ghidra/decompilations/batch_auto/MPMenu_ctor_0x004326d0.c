
void * __thiscall MPMenu_ctor(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  ulonglong uVar3;
  ulonglong uVar4;
  undefined4 in_stack_fffffbc4;
  undefined4 in_stack_fffffbc8;
  undefined4 in_stack_fffffbcc;
  undefined4 in_stack_fffffbd0;
  undefined4 uVar5;
  char local_40c [1024];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb57b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  SimpleMenu_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_TourneyMenu_DeletingDtor2_004d4208;
  *(char **)((int)this + 0x868) = "MP Menu";
  iVar2 = 0x550;
  do {
    iVar1 = *(int *)((int)this + 0xcac);
    uVar3 = Font_MeasureText((char *)**(undefined4 **)(iVar2 + *(int *)((int)this + 0x878)));
    uVar5 = 0x432757;
    uVar4 = Font_MeasureText("CONTROLLER 2:");
    if (iVar1 < (int)uVar4 + 0x14 + (int)uVar3) {
      uVar3 = Font_MeasureText((char *)**(undefined4 **)(iVar2 + *(int *)((int)this + 0x878)));
      uVar5 = 0x43278e;
      uVar4 = Font_MeasureText("CONTROLLER 2:");
      iVar1 = (int)uVar3 + 0x14 + (int)uVar4;
    }
    iVar2 = iVar2 + 4;
    *(int *)((int)this + 0xcac) = iVar1;
  } while (iVar2 < 0x560);
  iVar2 = 0;
  *(char **)((int)this + 0x888) = "PARTY GAMES!";
  *(undefined4 *)((int)this + 0x880) = *(undefined4 *)(*(int *)((int)this + 0x878) + 800);
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"PARTY RACE (2P ONLY)","2PRACE",in_stack_fffffbc4,in_stack_fffffbc8,
                 in_stack_fffffbcc,in_stack_fffffbd0,uVar5,iVar2);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"RODENT RUMBLE (1-4P)","RUMBLE",in_stack_fffffbc4,in_stack_fffffbc8,
                 in_stack_fffffbcc,in_stack_fffffbd0,uVar5,iVar2);
  uVar5 = 0x43283a;
  UIList_AddSpacer(this,10);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  UIList_AddItemWithFormat
            (this,"CONTROLLER 1:","CONTROL1",in_stack_fffffbc4,in_stack_fffffbc8,in_stack_fffffbcc,
             in_stack_fffffbd0,uVar5,iVar2);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  UIList_AddItemWithFormat
            (this,"CONTROLLER 2:","CONTROL2",in_stack_fffffbc4,in_stack_fffffbc8,in_stack_fffffbcc,
             in_stack_fffffbd0,uVar5,iVar2);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  UIList_AddItemWithFormat
            (this,"CONTROLLER 3:","CONTROL3",in_stack_fffffbc4,in_stack_fffffbc8,in_stack_fffffbcc,
             in_stack_fffffbd0,uVar5,iVar2);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  UIList_AddItemWithFormat
            (this,"CONTROLLER 4:","CONTROL4",in_stack_fffffbc4,in_stack_fffffbc8,in_stack_fffffbcc,
             in_stack_fffffbd0,uVar5,iVar2);
  uVar5 = 0x432917;
  UIList_AddSpacer(this,10);
  iVar2 = 0;
  Matrix_Scale4x4(&stack0xfffffbc4,0x3f800000,0x3f400000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"PREVIOUS MENU","BACK",in_stack_fffffbc4,in_stack_fffffbc8,in_stack_fffffbcc,
                 in_stack_fffffbd0,uVar5,iVar2);
  AthenaString_SprintfToBuffer(local_40c,(byte *)"PLAYER 1: %s");
  UIList_SetTextByName(this,local_40c,"CONTROL1");
  OptionsMenu_RenderControls(this);
  OptionsMenu_UpdateControl2(this);
  OptionsMenu_UpdateControl3(this);
  OptionsMenu_UpdateControl4(this);
  ExceptionList = local_c;
  return this;
}

