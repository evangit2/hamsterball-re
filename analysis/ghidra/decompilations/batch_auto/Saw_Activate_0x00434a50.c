
void __fastcall Saw_Activate(int param_1)

{
  *(undefined1 *)(param_1 + 0x1114) = 1;
  Sound_Play3D(*(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x4c0),
               *(float *)(param_1 + 0x10d4),*(float *)(param_1 + 0x10d8),
               *(float *)(param_1 + 0x10dc));
  return;
}

