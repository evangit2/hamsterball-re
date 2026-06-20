/*
 * Function: Vec3_AddInPlace
 * Address: 0x004018f0
 * Signature: void __thiscall Vec3_AddInPlace(void *this,float *param_1)
 *
 * Patterns: none identified. Calls: Vec3_AddInPlace. Offsets: 0, Lines: 7
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_AddInPlace(void *this,float *param_1)

{
  *(float *)this = *param_1 + *(float *)this;
  *(float *)((int)this + 4) = param_1[1] + *(float *)((int)this + 4);
  *(float *)((int)this + 8) = param_1[2] + *(float *)((int)this + 8);
  return;
}
