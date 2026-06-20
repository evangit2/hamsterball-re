/*
 * Function: Ball_SetFlatGravity
 * Address: 0x00403150
 * Signature: void __fastcall Ball_SetFlatGravity(int param_1)
 *
 * Patterns: vtable dispatch, ball. Calls: Ball_SetFlatGravity. Offsets: 7, Lines: 16
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_SetFlatGravity(int param_1)

{
  int iVar1;
  undefined4 local_c [3];
  
  iVar1 = *(int *)(param_1 + 0x1a4);
  *(undefined4 *)(param_1 + 0x748) = 2;
  if ((undefined4 *)(iVar1 + 0xc8c) != local_c) {
    *(undefined4 *)(iVar1 + 0xc8c) = 0;
    *(undefined4 *)(iVar1 + 0xc90) = 0;
    *(undefined4 *)(iVar1 + 0xc94) = 0x3f800000;
  }
  (**(code **)(*(int *)(param_1 + 0x1c) + 4))();
  *(undefined4 *)(param_1 + 0xf8) = *(undefined4 *)(param_1 + 0x748);
  return;
}
