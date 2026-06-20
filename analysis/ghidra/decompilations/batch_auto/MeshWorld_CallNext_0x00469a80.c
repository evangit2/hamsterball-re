
void __fastcall MeshWorld_CallNext(int param_1)

{
  if (*(int **)(param_1 + 0x424) != (int *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00469a8c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(int **)(param_1 + 0x424) + 0x24))();
    return;
  }
  return;
}

