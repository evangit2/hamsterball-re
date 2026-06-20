/*
 * Function: Ball_ResetCollisionMesh
 * Address: 0x004030b0
 * Signature: void __fastcall Ball_ResetCollisionMesh(int param_1)
 *
 * Patterns: vtable dispatch, collision, ball. Calls: Ball_ResetCollisionMesh. Offsets: 7, Lines: 16
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_ResetCollisionMesh(int param_1)

{
  int iVar1;
  undefined4 local_c [3];
  
  iVar1 = *(int *)(param_1 + 0x1a4);
  *(undefined4 *)(param_1 + 0x748) = 0;
  if ((undefined4 *)(iVar1 + 0xc8c) != local_c) {
    *(undefined4 *)(iVar1 + 0xc8c) = 0;
    *(undefined4 *)(iVar1 + 0xc90) = 0xbf800000;
    *(undefined4 *)(iVar1 + 0xc94) = 0;
  }
  (**(code **)(*(int *)(param_1 + 0x1c) + 4))();
  *(undefined4 *)(param_1 + 0xf8) = *(undefined4 *)(param_1 + 0x748);
  return;
}
