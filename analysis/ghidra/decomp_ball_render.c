
void __fastcall Ball_Render(void *param_1)

{
  int *piVar1;
  undefined4 uVar2;
  
  if (*(char *)((int)param_1 + 0x70d) != '\x01') {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),7,1);
    *(undefined1 *)((int)param_1 + 0x70d) = 1;
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
  }
  if (*(int *)((int)param_1 + 0x6fc) != 1) {
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
    *(undefined4 *)((int)param_1 + 0x6fc) = 1;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0x130))(*(int **)((int)param_1 + 0x154),0x112);
    FUN_00401090(param_1,*(int *)((int)param_1 + 0x708),'\0');
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x1d,1);
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),7,1);
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x89,1);
    piVar1 = *(int **)((int)param_1 + 0x154);
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
    if (*(char *)((int)param_1 + 0x700) == '\x01') {
      (**(code **)(*piVar1 + 200))(piVar1,0x1b,1);
      uVar2 = 4;
    }
    else {
      (**(code **)(*piVar1 + 200))(piVar1,0x1b,0);
      uVar2 = 2;
    }
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))
              (*(int **)((int)param_1 + 0x154),0,4,uVar2);
    FUN_00453970(param_1,*(undefined1 *)((int)param_1 + 0x734));
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))
              (*(int **)((int)param_1 + 0x154),0x34,*(undefined1 *)((int)param_1 + 0x182));
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x38,8);
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x37,3);
  }
  return;
}

