
void * __thiscall RaceResultPopup_ctor(void *this,int param_1,undefined4 param_2)

{
  int iVar1;
  void *pvVar2;
  char *pcVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cc60e;
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
  *(undefined ***)this = &PTR_RaceGoalReached_ScalarDtor_004d6ca0;
  ToggleTimer_Init((undefined4 *)((int)this + 0x3c));
  iVar4 = *(int *)((int)this + 8);
  local_4._0_1_ = 1;
  *(undefined4 *)((int)this + 0x50) = 0;
  piVar6 = &DAT_004f710c;
  iVar5 = 2;
  do {
    if ((float)piVar6[-1] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[-1] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + -2;
    }
    if ((float)*piVar6 < *(float *)(iVar4 + 0x18) != ((float)*piVar6 == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + -1;
    }
    if ((float)piVar6[1] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[1] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5;
    }
    if ((float)piVar6[2] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[2] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + 1;
    }
    if ((float)piVar6[3] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[3] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + 2;
    }
    if ((float)piVar6[4] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[4] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + 3;
    }
    if ((float)piVar6[5] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[5] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + 4;
    }
    if ((float)piVar6[6] < *(float *)(iVar4 + 0x18) !=
        ((float)piVar6[6] == *(float *)(iVar4 + 0x18))) {
      *(int *)((int)this + 0x50) = iVar5 + 5;
    }
    iVar1 = iVar5 + 6;
    piVar6 = piVar6 + 8;
    iVar5 = iVar5 + 8;
  } while (iVar1 < 0x10);
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pcVar3 = (char *)AthenaString_Format(0x4f7448,(byte *)"textures\\ranks\\%d.jpg");
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)(*(int *)((int)this + 4) + 0x878) + 0x174),pcVar3
                        );
  }
  local_4 = CONCAT31(local_4._1_3_,1);
  *(void **)((int)this + 0x54) = pvVar2;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(undefined4 *)((int)this + 0x20) = 800;
  iVar4 = RNG_Rand(&PTR_OBJ_VTABLE,2,'\0');
  if (iVar4 == 0) {
    *(char **)((int)this + 0x24) = "TIME\'S UP!";
  }
  else if (iVar4 == 1) {
    *(char **)((int)this + 0x24) = "OUT OF TIME!";
  }
  *(undefined4 *)((int)this + 0x2c) = 800;
  *(undefined4 *)((int)this + 0x34) = 800;
  *(undefined4 *)((int)this + 0x38) = 800;
  *(undefined1 *)((int)this + 0x58) = 0;
  *(undefined1 *)((int)this + 0x59) = 0;
  *(undefined4 *)((int)this + 0x5c) = 0;
  *(undefined4 *)((int)this + 0x28) = 0x3f800000;
  *(undefined4 *)((int)this + 0x30) = 0xfffffce0;
  ExceptionList = local_c;
  return this;
}

