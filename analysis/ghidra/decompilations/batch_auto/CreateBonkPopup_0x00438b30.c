
void __fastcall CreateBonkPopup(int param_1)

{
  if (*(char *)(param_1 + 0x10fc) != '\0') {
    *(undefined1 *)(param_1 + 0x10fc) = 0;
    Sound_Play3D(*(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x45c),
                 *(float *)(param_1 + 0x10d4),*(float *)(param_1 + 0x10d8),
                 *(float *)(param_1 + 0x10dc));
    (**(code **)(**(int **)(param_1 + 0x10d0) + 0x88))("BONKPOPUP");
  }
  return;
}

