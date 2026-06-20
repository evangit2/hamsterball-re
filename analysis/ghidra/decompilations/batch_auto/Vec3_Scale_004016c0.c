/*
 * Function: Vec3_Scale
 * Address: 0x004016c0
 * Signature: void __thiscall Vec3_Scale(void *this,float *param_1,float param_2)
 *
 * Patterns: none identified. Calls: Vec3_Scale. Offsets: 0, Lines: 12
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_Scale(void *this,float *param_1,float param_2)

{
  float fVar1;
  float fVar2;
  
  fVar1 = *(float *)((int)this + 8);
  fVar2 = *(float *)((int)this + 4);
  *param_1 = param_2 * *(float *)this;
  param_1[1] = param_2 * fVar2;
  param_1[2] = param_2 * fVar1;
  return;
}
