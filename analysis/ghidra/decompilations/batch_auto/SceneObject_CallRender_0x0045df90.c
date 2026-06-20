
void __fastcall SceneObject_CallRender(int param_1)

{
  if (*(int **)(param_1 + 0x434) != (int *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0045df9c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(int **)(param_1 + 0x434) + 0xc))();
    return;
  }
  return;
}

