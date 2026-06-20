/*
 * Function: Ball_StartFall
 * Address: 0x00402200
 * Signature: void __fastcall Ball_StartFall(int param_1)
 *
 * Patterns: audio, ball. Calls: Ball_StartFall, Sound_Play3D. Offsets: 8, Lines: 9
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_StartFall(int param_1)

{
  *(undefined1 *)(param_1 + 0xc4c) = 1;
  *(undefined4 *)(param_1 + 0x284) = 0x41500000;
  *(undefined4 *)(param_1 + 0x188) = 0x40200000;
  Sound_Play3D(*(void **)(*(int *)(param_1 + 0x10) + 0x4d4),*(float *)(param_1 + 0x164),
               *(float *)(param_1 + 0x168),*(float *)(param_1 + 0x16c));
  return;
}
