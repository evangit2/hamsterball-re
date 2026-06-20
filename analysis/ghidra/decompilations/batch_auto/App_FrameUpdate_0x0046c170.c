
void __fastcall App_FrameUpdate(int param_1)

{
  HWND pHVar1;
  HWND pHVar2;
  POINT Point;
  HWND pHVar3;
  tagPOINT local_8;
  
  GetCursorPos(&local_8);
  Point.y = local_8.y;
  Point.x = local_8.x;
  pHVar3 = WindowFromPoint(Point);
  pHVar1 = *(HWND *)(param_1 + 0xc);
  *(HWND *)(param_1 + 0xc) = pHVar3;
  if (pHVar3 == pHVar1) goto LAB_0046c1b9;
  pHVar2 = *(HWND *)(param_1 + 8);
  if (pHVar3 == pHVar2) {
LAB_0046c1aa:
    if (pHVar1 == pHVar2) goto LAB_0046c1b9;
  }
  else if (pHVar1 != pHVar2) {
    if (pHVar3 != pHVar2) goto LAB_0046c1b9;
    goto LAB_0046c1aa;
  }
  MeshWorld_ClearCurrent(*(int *)(param_1 + 0x184));
LAB_0046c1b9:
  if (*(int *)(param_1 + 0x178) != 0) {
    SoundDevice_UpdateChannels(*(int *)(param_1 + 0x178));
  }
  if (*(int *)(param_1 + 0x17c) != 0) {
    MusicDevice_FadeAll(*(int *)(param_1 + 0x17c));
  }
  if (*(int *)(param_1 + 0x180) != 0) {
    InputDevice_PollAndRelease(*(int *)(param_1 + 0x180));
  }
  GameUpdate(*(int *)(param_1 + 0x184));
  return;
}

