
void __fastcall Input_OnMouseUpCapture(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 400) + -1;
  *(int *)(param_1 + 400) = iVar1;
  if (iVar1 < 1) {
    *(undefined4 *)(param_1 + 400) = 0;
    ReleaseCapture();
  }
  *(undefined4 *)(param_1 + 0x188) = 0;
  *(undefined1 *)(param_1 + 0x1c8) = 0;
  *(undefined1 *)(param_1 + 0x1c9) = 0;
  *(undefined1 *)(param_1 + 0x1ca) = 0;
  if (*(int **)(param_1 + 0x1b0) != (int *)0x0) {
    *(char **)(param_1 + 0x210) = "MouseUp";
    *(char **)(param_1 + 0x20c) = "Mouse Interceptor";
                    /* WARNING: Could not recover jumptable at 0x0046c41f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(int **)(param_1 + 0x1b0) + 0x10))();
    return;
  }
  return;
}

