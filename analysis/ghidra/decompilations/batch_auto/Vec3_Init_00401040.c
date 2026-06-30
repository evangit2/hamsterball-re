/*
 * Function: Vec3_Init
 * Address: 0x00401040
 * Signature: void __thiscall Vec3_Init(void *this, int param_1)
 *
 * Description:
 * Initializes a Vec3 object by:
 *   1. Setting the vtable pointer at offset +0x00 to PTR_Vec3_dtor_004cf300
 *      (this is the Vec3/Matrix class vtable, whose first entry is Vec3_dtor)
 *   2. Copying 4 float components (x, y, z, w) from the source (param_1+4 through param_1+0x10)
 *      into the destination (this+4 through this+0x10)
 *
 * Despite the name "Vec3", this copies 4 components (16 bytes), meaning the object
 * is actually a Vec4 or a matrix row. The vtable at 0x4CF300 contains Vec3_dtor as
 * its first entry (scalar deleting destructor), confirming this is a proper C++ class.
 *
 * Cross-references:
 *   - Called from LevelBoard_Odd_dtor (0x41EE70) at address 0x41EFD4
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_Init(void *this,int param_1)

{
  *(undefined ***)this = &PTR_Vec3_dtor_004cf300;
  *(undefined4 *)((int)this + 4) = *(undefined4 *)(param_1 + 4);
  *(undefined4 *)((int)this + 8) = *(undefined4 *)(param_1 + 8);
  *(undefined4 *)((int)this + 0xc) = *(undefined4 *)(param_1 + 0xc);
  *(undefined4 *)((int)this + 0x10) = *(undefined4 *)(param_1 + 0x10);
  return;
}
