
void __fastcall CollisionLevel_PlayBreakSound(int param_1)

{
  Sound_Play3D(*(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x4b4),
               *(float *)(param_1 + 0x10d8),*(float *)(param_1 + 0x10dc),
               *(float *)(param_1 + 0x10e0));
  *(undefined4 *)(param_1 + 0x10e4) = 0x3f800000;
  return;
}

