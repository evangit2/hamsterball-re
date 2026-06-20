
void __fastcall Gfx_SetupAlphaRenderState(void *param_1)

{
  int *piVar1;
  undefined4 uVar2;
  
  if (*(int *)((int)param_1 + 0x6fc) != 2) {
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
    *(undefined4 *)((int)param_1 + 0x6fc) = 2;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0x130))(*(int **)((int)param_1 + 0x154),0x152);
    piVar1 = *(int **)((int)param_1 + 0x154);
    if (*(char *)((int)param_1 + 0x7d2) == '\0') {
      (**(code **)(*piVar1 + 200))(piVar1,0x16,3);
    }
    else {
      (**(code **)(*piVar1 + 200))(piVar1,0x16,2);
    }
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x1d,0);
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),7,1);
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))(*(int **)((int)param_1 + 0x154),0x89,0);
    Graphics_SetCullMode2(param_1,*(undefined1 *)((int)param_1 + 0x734));
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
  }
  return;
}

