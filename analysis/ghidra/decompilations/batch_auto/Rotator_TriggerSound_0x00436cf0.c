
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Rotator_TriggerSound(int param_1)

{
  float fVar1;
  
  if (*(char *)(param_1 + 0x10e4) == '\0') {
    fVar1 = *(float *)(param_1 + 0x10d8) - _DAT_004cf370;
    *(undefined1 *)(param_1 + 0x10e4) = 1;
    *(undefined1 *)(param_1 + 0x10e5) = 1;
    *(float *)(param_1 + 0x10d8) = fVar1;
    Sound_Play3D(*(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x510),
                 *(float *)(param_1 + 0x10d4),*(float *)(param_1 + 0x10d8),
                 *(float *)(param_1 + 0x10dc));
  }
  return;
}

