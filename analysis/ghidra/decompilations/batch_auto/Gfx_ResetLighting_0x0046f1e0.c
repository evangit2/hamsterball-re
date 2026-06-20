
void __fastcall Gfx_ResetLighting(int param_1)

{
  if (*(char *)(param_1 + 0x700) != '\0') {
    *(int *)(param_1 + 0x7c8) = *(int *)(param_1 + 0x7c8) + 1;
    *(undefined1 *)(param_1 + 0x700) = 0;
    (**(code **)(**(int **)(param_1 + 0x154) + 200))(*(int **)(param_1 + 0x154),0x1b,0);
    (**(code **)(**(int **)(param_1 + 0x154) + 0xfc))(*(int **)(param_1 + 0x154),0,4,2);
  }
  (**(code **)(**(int **)(param_1 + 0x154) + 0xf4))(*(int **)(param_1 + 0x154),0,0);
  (**(code **)(**(int **)(param_1 + 0x154) + 200))(*(int **)(param_1 + 0x154),0x39,0);
  return;
}

