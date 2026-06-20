
void * __thiscall KeyboardDevice_Ctor(void *this,int param_1,int *param_2)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  
  *(undefined ***)this = &PTR_KeyboardDevice_ScalarDtor_004d9840;
  *(int *)((int)this + 4) = param_1;
  *(int **)((int)this + 8) = param_2;
  (**(code **)(*param_2 + 0x2c))(param_2,&DAT_004daf3c);
  (**(code **)(**(int **)((int)this + 8) + 0x34))
            (*(int **)((int)this + 8),*(undefined4 *)(*(int *)(*(int *)((int)this + 4) + 4) + 8),6);
  (**(code **)(**(int **)((int)this + 8) + 0x1c))(*(int **)((int)this + 8));
  puVar4 = (undefined4 *)((int)this + 0xc);
  for (iVar3 = 0x40; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  puVar4 = (undefined4 *)((int)this + 0x10c);
  for (iVar3 = 0xff; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = &DAT_004d9bf8;
    puVar4 = puVar4 + 1;
  }
  *(undefined4 **)((int)this + 0x138) = &DAT_004d435c;
  *(undefined **)((int)this + 0x114) = &DAT_004cfa24;
  *(undefined **)((int)this + 0x118) = &DAT_004cfa20;
  *(undefined **)((int)this + 0x11c) = &DAT_004d4358;
  *(undefined **)((int)this + 0x120) = &DAT_004d4344;
  *(undefined **)((int)this + 0x124) = &DAT_004d4330;
  *(undefined **)((int)this + 0x128) = &DAT_004d4320;
  *(undefined **)((int)this + 300) = &DAT_004d4310;
  *(undefined **)((int)this + 0x130) = &DAT_004d42fc;
  *(undefined **)((int)this + 0x134) = &DAT_004d42ec;
  *(undefined **)((int)this + 0x184) = &DAT_004d9bf4;
  *(undefined **)((int)this + 0x1cc) = &DAT_004d9bf0;
  *(undefined **)((int)this + 0x1c4) = &DAT_004d9bec;
  *(undefined **)((int)this + 0x18c) = &DAT_004d9be8;
  *(undefined **)((int)this + 0x154) = &DAT_004d9be4;
  *(undefined **)((int)this + 400) = &DAT_004d9be0;
  *(undefined **)((int)this + 0x194) = &DAT_004d9bdc;
  *(undefined **)((int)this + 0x198) = &DAT_004d9bd8;
  *(undefined **)((int)this + 0x168) = &DAT_004d9bd4;
  *(undefined **)((int)this + 0x19c) = &DAT_004d9bd0;
  *(undefined **)((int)this + 0x1a0) = &DAT_004d9bcc;
  *(undefined **)((int)this + 0x1a4) = &DAT_004d9bc8;
  *(undefined **)((int)this + 0x1d4) = &DAT_004d9bc4;
  *(undefined **)((int)this + 0x1d0) = &DAT_004d9bc0;
  *(undefined **)((int)this + 0x16c) = &DAT_004d9bbc;
  *(undefined **)((int)this + 0x170) = &DAT_004d9bb8;
  *(undefined **)((int)this + 0x14c) = &DAT_004d9bb4;
  *(undefined **)((int)this + 0x158) = &DAT_004d9bb0;
  *(undefined **)((int)this + 0x188) = &DAT_004d9bac;
  *(undefined **)((int)this + 0x15c) = &DAT_004d9ba8;
  *(undefined **)((int)this + 0x164) = &DAT_004d9ba4;
  *(undefined **)((int)this + 0x1c8) = &DAT_004d9ba0;
  *(undefined **)((int)this + 0x150) = &DAT_004d9b9c;
  *(undefined **)((int)this + 0x1c0) = &DAT_004cf854;
  *(undefined **)((int)this + 0x160) = &DAT_004cf850;
  *(undefined **)((int)this + 0x1bc) = &DAT_004cf84c;
  *(undefined **)((int)this + 0x1ac) = &DAT_004d9b98;
  *(char **)((int)this + 0x480) = "APPLICATION";
  *(undefined **)((int)this + 0x1b8) = &DAT_004d9b88;
  *(char **)((int)this + 0x144) = "BACKSPACE";
  *(char **)((int)this + 0x390) = "CALCULATOR";
  *(char **)((int)this + 500) = "CAPS LOCK";
  *(undefined **)((int)this + 0x1d8) = &DAT_004d9b60;
  *(char **)((int)this + 0x458) = "DELETE";
  *(char **)((int)this + 0x44c) = "DOWN ARROW";
  *(char **)((int)this + 0x42c) = "UP ARROW";
  *(char **)((int)this + 0x438) = "LEFT ARROW";
  *(char **)((int)this + 0x440) = "RIGHT ARROW";
  *(undefined **)((int)this + 0x448) = &DAT_004d9b24;
  *(undefined **)((int)this + 0x140) = &DAT_004d9b20;
  *(undefined **)((int)this + 0x110) = &DAT_004d9b1c;
  *(undefined **)((int)this + 0x1f8) = &DAT_004d9b18;
  *(undefined **)((int)this + 0x1fc) = &DAT_004d9b14;
  *(undefined **)((int)this + 0x200) = &DAT_004d9b10;
  *(undefined **)((int)this + 0x204) = &DAT_004d9b0c;
  *(undefined **)((int)this + 0x208) = &DAT_004d9b08;
  *(undefined **)((int)this + 0x20c) = &DAT_004d9b04;
  *(undefined **)((int)this + 0x210) = &DAT_004d9b00;
  *(undefined **)((int)this + 0x214) = &DAT_004d9afc;
  *(undefined **)((int)this + 0x218) = &DAT_004d9af8;
  *(undefined **)((int)this + 0x21c) = &DAT_004d9af4;
  *(undefined **)((int)this + 0x268) = &DAT_004d9af0;
  *(undefined **)((int)this + 0x26c) = &DAT_004d9aec;
  *(undefined **)((int)this + 0x29c) = &DAT_004d9ae8;
  *(undefined **)((int)this + 0x2a0) = &DAT_004d9ae4;
  *(undefined **)((int)this + 0x2a4) = &DAT_004d9ae0;
  *(undefined **)((int)this + 0x1b0) = &DAT_004d9adc;
  *(undefined **)((int)this + 0x428) = &DAT_004cf73c;
  *(char **)((int)this + 0x454) = "INSERT";
  *(char **)((int)this + 0x1ec) = "LEFT ALT";
  *(char **)((int)this + 0x3ec) = "RIGHT ALT";
  *(undefined **)((int)this + 0x174) = &DAT_004d9ab8;
  *(undefined **)((int)this + 0x178) = &DAT_004d9ab4;
  *(char **)((int)this + 0x180) = "LEFT CTRL";
  *(char **)((int)this + 0x380) = "RIGHT CTRL";
  *(char **)((int)this + 0x1b4) = "LEFT SHIFT";
  *(char **)((int)this + 0x1e4) = "RIGHT SHIFT";
  *(char **)((int)this + 0x478) = "LEFT WIN";
  *(char **)((int)this + 0x47c) = "RIGHT WIN";
  *(undefined **)((int)this + 0x4bc) = &DAT_004d9a64;
  *(char **)((int)this + 0x4c0) = "MEDIA SELECT";
  *(char **)((int)this + 0x39c) = "MEDIA STOP";
  *(undefined **)((int)this + 0x13c) = &DAT_004d9a44;
  *(char **)((int)this + 0x1e8) = "* (KEYPAD)";
  *(undefined **)((int)this + 0x38c) = &DAT_004d9a30;
  *(char **)((int)this + 0x4b8) = "MY COMPUTER";
  *(char **)((int)this + 0x370) = "NEXT TRACK";
  *(char **)((int)this + 0x220) = "NUMLOCK";
  *(char **)((int)this + 0x248) = "1 (KEYPAD)";
  *(char **)((int)this + 0x24c) = "2 (KEYPAD)";
  *(char **)((int)this + 0x250) = "3 (KEYPAD)";
  *(char **)((int)this + 0x238) = "4 (KEYPAD)";
  *(char **)((int)this + 0x23c) = "5 (KEYPAD)";
  *(char **)((int)this + 0x240) = "6 (KEYPAD)";
  *(char **)((int)this + 0x228) = "7 (KEYPAD)";
  *(char **)((int)this + 0x22c) = "8 (KEYPAD)";
  *(char **)((int)this + 0x230) = "9 (KEYPAD)";
  *(char **)((int)this + 0x254) = "0 (KEYPAD)";
  *(char **)((int)this + 0x3d8) = ", (KEYPAD)";
  *(char **)((int)this + 0x37c) = "ENTER (KEYPAD)";
  *(char **)((int)this + 0x340) = "= (KEYPAD)";
  *(char **)((int)this + 0x234) = "- (KEYPAD)";
  *(char **)((int)this + 600) = ". (KEYPAD)";
  *(char **)((int)this + 0x244) = "+ (KEYPAD)";
  *(char **)((int)this + 0x3e0) = "/ (KEYPAD)";
  *(char **)((int)this + 0x420) = "PAUSE";
  *(undefined **)((int)this + 0x1dc) = &DAT_004d9934;
  *(char **)((int)this + 0x450) = "PAGE DOWN";
  *(char **)((int)this + 0x430) = "PAGE UP";
  *(char **)((int)this + 0x394) = "PAUSE";
  *(char **)((int)this + 0x484) = "POWER";
  *(char **)((int)this + 0x34c) = "PREVIOUS";
  *(char **)((int)this + 0x17c) = "ENTER";
  *(char **)((int)this + 0x224) = "SCROLL LOCK";
  *(undefined **)((int)this + 0x1a8) = &DAT_004d9900;
  *(undefined **)((int)this + 0x1e0) = &DAT_004d98fc;
  *(char **)((int)this + 0x488) = "SLEEP";
  *(char **)((int)this + 0x1f0) = "SPACE";
  *(undefined ***)((int)this + 0x148) = &PTR_LAB_004d98e8;
  *(char **)((int)this + 0x3c4) = "VOLUME DOWN";
  *(char **)((int)this + 0x3cc) = "VOLUME UP";
  *(undefined **)((int)this + 0x498) = &DAT_004d98c8;
  *(char **)((int)this + 0x4b4) = "WEB BACK";
  *(char **)((int)this + 0x4a4) = "FAVORITES";
  *(char **)((int)this + 0x4b0) = "WEB FORWARD";
  *(char **)((int)this + 0x3d4) = "WEB HOME";
  *(char **)((int)this + 0x4a8) = "WEB REFRESH";
  *(char **)((int)this + 0x4a0) = "WEB SEARCH";
  *(char **)((int)this + 0x4ac) = "WEB STOP";
  *(undefined4 *)((int)this + 0x50c) = 200;
  *(undefined4 *)((int)this + 0x510) = 0xd0;
  *(undefined4 *)((int)this + 0x514) = 0xcb;
  *(undefined4 *)((int)this + 0x518) = 0xcd;
  *(undefined4 *)((int)this + 0x51c) = 0x1c;
  *(undefined4 *)((int)this + 0x520) = 0x39;
  RegKey_Open(*(int *)(*(int *)(param_1 + 4) + 0x54));
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyUp");
  if (cVar1 != '\0') {
    uVar2 = RegKey_ReadDword(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyUp");
    *(uint *)((int)this + 0x50c) = uVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyDown");
  if (cVar1 != '\0') {
    uVar2 = RegKey_ReadDword(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyDown");
    *(uint *)((int)this + 0x510) = uVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyLeft");
  if (cVar1 != '\0') {
    uVar2 = RegKey_ReadDword(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyLeft");
    *(uint *)((int)this + 0x514) = uVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyRight");
  if (cVar1 != '\0') {
    uVar2 = RegKey_ReadDword(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyRight");
    *(uint *)((int)this + 0x518) = uVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyAction1");
  if (cVar1 != '\0') {
    uVar2 = RegKey_ReadDword(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyAction1");
    *(uint *)((int)this + 0x51c) = uVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyAction2");
  if (cVar1 != '\0') {
    uVar2 = RegKey_ReadDword(*(void **)(*(int *)(param_1 + 4) + 0x54),"KeyAction2");
    *(uint *)((int)this + 0x520) = uVar2;
  }
  RegKey_Close(*(int *)(*(int *)(param_1 + 4) + 0x54));
  return this;
}

