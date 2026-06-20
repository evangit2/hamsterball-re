/*
 * Function: Graphics_SetViewportClip
 * Address: 0x00401160
 * Signature: void __thiscall Graphics_SetViewportClip(void *this, ...)
 *
 * Description:
 * Sets a viewport clipping rectangle on the Graphics object.
Sets this+0x7A8=1 (clip enabled flag), then creates a 4x4 scaling matrix from
params (width, height, depth?, w?) and stores it at this+0x7B0-0x7BC (4 floats = matrix row).
Resets a temp matrix to identity afterward.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __thiscall
Graphics_SetViewportClip(void *this, undefined4 param_1, undefined4 param_2,
                         undefined4 param_3, undefined4 param_4)
{
  int iVar1;
  undefined4 local_14 [5];
  *(undefined1 *)((int)this + 0x7a8) = 1;
  iVar1 = Matrix_Scale4x4(local_14, param_1, param_2, param_3, param_4);
  *(undefined4 *)((int)this + 0x7b0) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x7b4) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x7b8) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x7bc) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_14);
  return;
}
