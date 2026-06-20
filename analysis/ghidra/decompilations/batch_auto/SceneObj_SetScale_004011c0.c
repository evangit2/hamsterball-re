/*
 * Function: SceneObj_SetScale
 * Address: 0x004011c0
 * Signature: void __thiscall SceneObj_SetScale(void *this, undefined4 param_1)
 *
 * Description:
 * Sets uniform scale on a SceneObject. Sets this+0x7A8=1 (dirty flag),
then creates a scale matrix with XYZ=(1.0, 1.0, 1.0, param_1) — param_1 is the scale
factor applied to all axes uniformly (0x3f800000 = 1.0f for x,y,z).
Stores the scale matrix row at this+0x7B0-0x7BC.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __thiscall SceneObj_SetScale(void *this, undefined4 param_1)
{
  int iVar1;
  undefined4 local_14 [5];
  *(undefined1 *)((int)this + 0x7a8) = 1;
  iVar1 = Matrix_Scale4x4(local_14, 0x3f800000, 0x3f800000, 0x3f800000, param_1);
  *(undefined4 *)((int)this + 0x7b0) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x7b4) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x7b8) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x7bc) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_14);
  return;
}
