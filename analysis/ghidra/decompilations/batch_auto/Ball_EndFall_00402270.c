/*
 * Function: Ball_EndFall
 * Address: 0x00402270
 * Signature: void __fastcall Ball_EndFall(int param_1)
 *
 * Patterns: ball. Calls: Ball_EndFall. Offsets: 3, Lines: 7
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_EndFall(int param_1)

{
  *(undefined1 *)(param_1 + 0xc4c) = 0;
  *(undefined4 *)(param_1 + 0x284) = 0x41d00000;
  *(undefined4 *)(param_1 + 0x188) = 0x40a00000;
  return;
}
