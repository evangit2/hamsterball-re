
void __fastcall Hammer_ChaseStart(int param_1)

{
  if (*(char *)(param_1 + 0x10fd) == '\0') {
    *(undefined1 *)(param_1 + 0x10fd) = 1;
    *(undefined4 *)(param_1 + 0x1104) = 1;
    if ((undefined4 *)(param_1 + 0x1108) != (undefined4 *)(param_1 + 0x10d4)) {
      *(undefined4 *)(param_1 + 0x1108) = *(undefined4 *)(param_1 + 0x10d4);
      *(undefined4 *)(param_1 + 0x110c) = *(undefined4 *)(param_1 + 0x10d8);
      *(undefined4 *)(param_1 + 0x1110) = *(undefined4 *)(param_1 + 0x10dc);
    }
    if ((undefined4 *)(param_1 + 0x1120) != (undefined4 *)(param_1 + 0x10e0)) {
      *(undefined4 *)(param_1 + 0x1120) = *(undefined4 *)(param_1 + 0x10e0);
      *(undefined4 *)(param_1 + 0x1124) = *(undefined4 *)(param_1 + 0x10e4);
      *(undefined4 *)(param_1 + 0x1128) = *(undefined4 *)(param_1 + 0x10e8);
    }
    *(undefined4 *)(param_1 + 0x1138) = 0x3f000000;
  }
  return;
}

