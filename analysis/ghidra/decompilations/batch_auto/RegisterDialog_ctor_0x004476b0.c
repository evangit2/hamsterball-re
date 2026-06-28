
void * __thiscall RegisterDialog_ctor(void *this,int param_1,byte *param_2)

{
  undefined4 *puVar1;
  void *this_00;
  char *pcVar2;
  char *pcVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc2f8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Gadget_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_ScoreDisplay_scalar_dtor_004d67e8;
  AthenaString_Init((undefined4 *)((int)this + 0x107c));
  puVar1 = (undefined4 *)((int)this + 0x1098);
  local_4._0_1_ = 1;
  AthenaString_Init(puVar1);
  local_4._0_1_ = 2;
  AthenaString_Init((undefined4 *)((int)this + 0x10b4));
  local_4._0_1_ = 3;
  AthenaString_Init((undefined4 *)((int)this + 0x10d0));
  local_4._0_1_ = 4;
  AthenaString_Init((undefined4 *)((int)this + 0x10ec));
  local_4._0_1_ = 5;
  ToggleTimer_Init((undefined4 *)((int)this + 0x110c));
  *(undefined4 *)((int)this + 0x1138) = 0xf;
  *(undefined4 *)((int)this + 0x1134) = 0;
  *(undefined1 *)((int)this + 0x1124) = 0;
  *(undefined4 *)((int)this + 0x1154) = 0xf;
  *(undefined4 *)((int)this + 0x1150) = 0;
  *(undefined1 *)((int)this + 0x1140) = 0;
  local_4 = CONCAT31(local_4._1_3_,8);
  pcVar2 = "Register Dialog";
  do {
    pcVar3 = pcVar2;
    pcVar2 = pcVar3 + 1;
  } while (*pcVar3 != '\0');
  StdString_Assign((void *)((int)this + 0x1120),(undefined4 *)"Register Dialog",
                   (uint)(pcVar3 + -0x4d683c));
  *(int *)((int)this + 0x878) = param_1;
  Font_WordWrap(*(void **)(param_1 + 0x31c),param_2,0x163,(byte *)((int)this + 0x87c));
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0x44480000;
  *(undefined4 *)((int)this + 0x10) = 0x44160000;
  *(undefined4 *)((int)this + 0x1080) = 0x43910000;
  *(undefined4 *)((int)this + 0x1084) = 0x43898000;
  *(undefined4 *)((int)this + 0x1088) = 0x435c0000;
  *(undefined4 *)((int)this + 0x108c) = 0x41f00000;
  AthenaList_Append((void *)((int)this + 0x44c),(int)this + 0x107c);
  pcVar2 = "";
  do {
    pcVar3 = pcVar2;
    pcVar2 = pcVar3 + 1;
  } while (*pcVar3 != '\0');
  StdString_Assign((void *)((int)this + 0x1120),(undefined4 *)&DAT_004d1354,
                   (uint)(pcVar3 + -0x4d1354));
  pcVar2 = "";
  do {
    pcVar3 = pcVar2;
    pcVar2 = pcVar3 + 1;
  } while (*pcVar3 != '\0');
  StdString_Assign((void *)((int)this + 0x113c),(undefined4 *)&DAT_004d1354,
                   (uint)(pcVar3 + -0x4d1354));
  *(undefined4 *)((int)this + 0x10a0) = 0x43d70000;
  *(undefined4 *)((int)this + 0x109c) = 0x43790000;
  *(undefined4 *)((int)this + 0x10a4) = 0x43988000;
  *(undefined4 *)((int)this + 0x10a8) = 0x41c80000;
  *(undefined1 *)((int)this + 0x10ad) = 1;
  *(undefined4 *)((int)this + 0x10c0) = 0x43988000;
  *(undefined4 *)((int)this + 0x10b8) = 0x43790000;
  *(undefined4 *)((int)this + 0x10bc) = 0x43f00000;
  *(undefined4 *)((int)this + 0x10c4) = 0x41c80000;
  *(undefined1 *)((int)this + 0x10c9) = 1;
  AthenaList_Append((void *)((int)this + 0x44c),(int)puVar1);
  this_00 = (void *)((int)this + 0x44c);
  AthenaList_Append(this_00,(int)this + 0x10b4);
  *(undefined4 **)((int)this + 0x1108) = puVar1;
  *(undefined4 *)((int)this + 0x1114) = 0x14;
  *(undefined4 *)((int)this + 0x10d4) = 0x43790000;
  *(undefined4 *)((int)this + 0x10d8) = 0x4400c000;
  *(undefined4 *)((int)this + 0x10dc) = 0x430c0000;
  *(undefined4 *)((int)this + 0x10e0) = 0x42200000;
  AthenaList_Append(this_00,(int)this + 0x10d0);
  *(undefined4 *)((int)this + 0x10f0) = 0x43cf0000;
  *(undefined4 *)((int)this + 0x10f4) = 0x4400c000;
  *(undefined4 *)((int)this + 0x10f8) = 0x430c0000;
  *(undefined4 *)((int)this + 0x10fc) = 0x42200000;
  AthenaList_Append(this_00,(int)this + 0x10ec);
  *(undefined4 *)((int)this + 0x1158) = 0;
  ExceptionList = local_c;
  return this;
}

