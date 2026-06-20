
void * __thiscall CreditsScreen_ctor(void *this,int param_1)

{
  void *this_00;
  undefined4 in_stack_ffffffc8;
  undefined4 in_stack_ffffffcc;
  undefined4 in_stack_ffffffd0;
  undefined4 in_stack_ffffffd4;
  undefined4 uVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004caba4;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  SimpleMenu_ctor(this,param_1);
  this_00 = (void *)((int)this + 0xce4);
  local_4 = 0;
  *(undefined ***)this = &PTR_CreditsScreen_scalar_dtor_004d2540;
  AthenaList_Init(this_00,0);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0x10fc),0);
  local_4 = CONCAT31(local_4._1_3_,2);
  *(char **)((int)this + 0x868) = "Credits Gadget";
  *(int *)((int)this + 0xcdc) = param_1;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0x44480000;
  *(undefined4 *)((int)this + 0x10) = 0x44160000;
  AthenaList_Append(this_00,0x4d2520);
  AthenaList_Append(this_00,0x4d2504);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d24f4);
  AthenaList_Append(this_00,0x4d24e8);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d24d8);
  AthenaList_Append(this_00,0x4d24cc);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d24c0);
  AthenaList_Append(this_00,0x4d24b4);
  AthenaList_Append(this_00,0x4d249c);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d2480);
  AthenaList_Append(this_00,0x4d24e8);
  AthenaList_Append(this_00,0x4d2474);
  AthenaList_Append(this_00,0x4d2460);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d2458);
  AthenaList_Append(this_00,0x4d2450);
  AthenaList_Append(this_00,0x4d2434);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d2424);
  AthenaList_Append(this_00,0x4d2414);
  AthenaList_Append(this_00,0x4d23fc);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d23ec);
  AthenaList_Append(this_00,0x4d23dc);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d23cc);
  AthenaList_Append(this_00,0x4d23bc);
  AthenaList_Append(this_00,0x4d23a8);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d2398);
  AthenaList_Append(this_00,0x4d238c);
  AthenaList_Append(this_00,0x4d237c);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d1354);
  AthenaList_Append(this_00,0x4d2364);
  AthenaList_Append(this_00,0x4d234c);
  AthenaList_Append(this_00,0x4d233c);
  AthenaList_Append(this_00,0x4d1354);
  uVar1 = 0x42576e;
  AthenaList_Append(this_00,0x4d1354);
  iVar2 = 0;
  *(undefined4 *)((int)this + 0xce0) = 0;
  Matrix_Scale4x4(&stack0xffffffc8,0x3f800000,0x3f400000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"DONE","BACK",in_stack_ffffffc8,in_stack_ffffffcc,in_stack_ffffffd0,
                 in_stack_ffffffd4,uVar1,iVar2);
  ExceptionList = local_c;
  return this;
}

