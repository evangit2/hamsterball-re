
void * __thiscall RaceResults_ctor(void *this,int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cc593;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)((int)this + 8) = param_2;
  *(undefined ***)this = &PTR_LAB_004d6c00;
  *(int *)((int)this + 4) = param_1;
  *(undefined4 *)((int)this + 0xc) = *(undefined4 *)(param_1 + 0x878);
  *(int *)(param_1 + 0x8b4) = *(int *)(param_1 + 0x8b4) + 1;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0x14) = 100;
  *(undefined1 *)((int)this + 0x18) = 0;
  *(undefined1 *)((int)this + 0x19) = 1;
  local_4 = 0;
  *(undefined ***)this = &PTR_ConfirmMenu_ScalarDtor_004d6c4c;
  ToggleTimer_Init((undefined4 *)((int)this + 0x74));
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined4 *)((int)this + 0x88) = 0;
  *(undefined1 *)((int)this + 0x54) = 0;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(undefined4 *)((int)this + 0x20) = 800;
  *(undefined4 *)((int)this + 0x5c) = 0;
  *(undefined4 *)((int)this + 0x60) = 0;
  *(undefined4 *)((int)this + 100) = 0;
  *(undefined4 *)((int)this + 0x68) = 0;
  *(undefined4 *)((int)this + 0x58) = 0;
  *(undefined1 *)((int)this + 0x55) = 0;
  *(undefined1 *)((int)this + 0x56) = 0;
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,5,'\0');
  switch(iVar2) {
  case 0:
    *(char **)((int)this + 0x24) = "YOU MADE IT!";
    break;
  case 1:
    *(char **)((int)this + 0x24) = "NICE GOING!";
    break;
  case 2:
    *(char **)((int)this + 0x24) = "HAMSTERIFFIC!";
    break;
  case 3:
    *(char **)((int)this + 0x24) = "GOAL!";
    break;
  case 4:
    *(char **)((int)this + 0x24) = "EXCELLENT!";
  }
  *(undefined4 *)((int)this + 0x3c) = 0x145;
  *(undefined4 *)((int)this + 0x40) = 0x145;
  *(undefined4 *)((int)this + 0x8c) = 0;
  *(undefined4 *)((int)this + 0x6c) = 0x44480000;
  *(undefined4 *)((int)this + 0x70) = 800;
  *(undefined4 *)((int)this + 0x28) = 100;
  *(undefined4 *)((int)this + 0x2c) = 0x7d;
  *(undefined4 *)((int)this + 0x30) = 200;
  *(undefined4 *)((int)this + 0x34) = 0xe1;
  *(undefined4 *)((int)this + 0x38) = 300;
  *(undefined4 *)((int)this + 0x44) = 0x15e;
  *(undefined4 *)((int)this + 0x48) = 0x177;
  *(undefined4 *)((int)this + 0x4c) = 400;
  *(undefined4 *)((int)this + 0x50) = 0x1a9;
  *(undefined4 *)((int)this + 0x14) = 0x2ee;
  *(undefined4 *)((int)this + 0x7c) = 0x32;
  uVar1 = *(undefined4 *)
           (*(int *)(*(int *)((int)this + 4) + 0x878) + 0x330 +
           *(int *)(*(int *)(*(int *)((int)this + 8) + 0x10) + 0x154) * 4);
  *(undefined1 *)((int)this + 0x94) = 0;
  *(undefined4 *)((int)this + 0x90) = uVar1;
  ExceptionList = local_c;
  return this;
}

