
void __fastcall MeshWorld_ClearCurrent(int param_1)

{
  if (*(int **)(param_1 + 0x41c) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x41c) + 0x30))();
  }
  *(undefined4 *)(param_1 + 0x41c) = 0;
  return;
}

