
void __fastcall Saw_AlertActivate(int param_1)

{
  if (*(char *)(param_1 + 0x110d) != '\0') {
    *(undefined1 *)(param_1 + 0x110d) = 0;
    Sound_Play3D(*(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x4bc),
                 *(float *)(param_1 + 0x10d4),*(float *)(param_1 + 0x10d8),
                 *(float *)(param_1 + 0x10dc));
  }
  return;
}

