
void __fastcall Gfx_SetBlendState(int param_1)

{
  if (*(int *)(param_1 + 0x704) != 1) {
    *(int *)(param_1 + 0x7c8) = *(int *)(param_1 + 0x7c8) + 1;
    (**(code **)(**(int **)(param_1 + 0x154) + 0xfc))(*(int **)(param_1 + 0x154),0,0xd,2);
    (**(code **)(**(int **)(param_1 + 0x154) + 0xfc))(*(int **)(param_1 + 0x154),0,0xe,2);
    *(undefined4 *)(param_1 + 0x704) = 1;
  }
  return;
}

