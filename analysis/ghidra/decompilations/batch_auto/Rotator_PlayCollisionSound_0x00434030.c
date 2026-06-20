
void __fastcall Rotator_PlayCollisionSound(int param_1)

{
  Sound_PlayChannel(*(int *)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x458));
  *(undefined1 *)(param_1 + 0x10d8) = 1;
  *(undefined4 *)(param_1 + 0x10ec) = 0x42480000;
  return;
}

