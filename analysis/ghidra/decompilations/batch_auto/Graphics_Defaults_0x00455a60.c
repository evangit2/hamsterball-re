
void __fastcall Graphics_Defaults(void *param_1)

{
  int *piVar1;
  
  piVar1 = *(int **)((int)param_1 + 0x154);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x38))(piVar1,*(undefined4 *)((int)param_1 + 400));
    Graphics_RenderScene(param_1);
    return;
  }
  return;
}

