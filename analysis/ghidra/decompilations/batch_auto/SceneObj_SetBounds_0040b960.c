/*
 * Function: SceneObj_SetBounds
 * Address: 0x0040b960
 * Signature: SceneObj_SetBounds(...)
 *
 * Patterns: matrix math, scene. Calls: SceneObj_SetBounds, Matrix_Identity. Offsets: 6, Lines: 22
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
SceneObj_SetBounds(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,
                  undefined4 param_4,undefined4 param_5)

{
  float fVar1;
  
  fVar1 = (float)_DAT_004cf3c8;
  *(undefined4 *)((int)this + 0x14) = param_2;
  *(undefined4 *)((int)this + 0x1c) = param_4;
  *(undefined4 *)((int)this + 0x18) = param_3;
  *(undefined4 *)((int)this + 0x20) = param_5;
  if (*(float *)((int)this + 0x10) != fVar1) {
    *(undefined1 *)((int)this + 0x4c) = 1;
    Matrix_Identity(&param_1);
    return;
  }
  *(undefined1 *)((int)this + 0x4c) = 0;
  Matrix_Identity(&param_1);
  return;
}
