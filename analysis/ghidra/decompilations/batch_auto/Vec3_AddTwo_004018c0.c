/*
 * Function: Vec3_AddTwo
 * Address: 0x004018c0
 * Signature: void __thiscall Vec3_AddTwo(void *this,float *param_1,float *param_2)
 *
 * Patterns: none identified. Calls: Vec3_AddTwo. Offsets: 0, Lines: 16
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_AddTwo(void *this,float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
  fVar1 = param_2[2];
  fVar2 = *(float *)((int)this + 8);
  fVar3 = param_2[1];
  fVar4 = *(float *)((int)this + 4);
  *param_1 = *param_2 + *(float *)this;
  param_1[1] = fVar3 + fVar4;
  param_1[2] = fVar1 + fVar2;
  return;
}
