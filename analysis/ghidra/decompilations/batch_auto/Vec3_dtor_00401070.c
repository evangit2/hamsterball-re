/*
 * Function: Vec3_dtor
 * Address: 0x00401070
 * Signature: void * __thiscall Vec3_dtor(void *this, byte param_1)
 *
 * Description:
 * Destructor for Vec3/Matrix object. Resets to identity matrix via Matrix_Identity(),
then conditionally frees the memory if param_1 & 1 (scalar deleting destructor pattern).
The 'Vec3' name is misleading — this operates on a matrix-sized object (4x4 = 64 bytes).
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void * __thiscall Vec3_dtor(void *this,byte param_1)
{
  Matrix_Identity(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
