
void __fastcall SoundDevice_ReleaseWindow(int param_1)

{
  if (*(int *)(param_1 + 0x45c) != 0) {
    Ordinal_108(*(int *)(param_1 + 0x45c));
  }
  if (*(UINT_PTR *)(param_1 + 0x474) != 0) {
    KillTimer(*(HWND *)(param_1 + 0x610),*(UINT_PTR *)(param_1 + 0x474));
  }
  if (*(UINT_PTR *)(param_1 + 0x478) != 0) {
    KillTimer(*(HWND *)(param_1 + 0x610),*(UINT_PTR *)(param_1 + 0x478));
  }
  if (*(int *)(param_1 + 0x460) != -1) {
    Ordinal_3(*(int *)(param_1 + 0x460));
  }
  DestroyWindow(*(HWND *)(param_1 + 0x610));
  *(undefined4 *)(param_1 + 0x610) = 0;
  *(undefined4 *)(param_1 + 0x45c) = 0;
  *(undefined4 *)(param_1 + 0x474) = 0;
  *(undefined4 *)(param_1 + 0x478) = 0;
  *(undefined4 *)(param_1 + 0x460) = 0xffffffff;
  return;
}

