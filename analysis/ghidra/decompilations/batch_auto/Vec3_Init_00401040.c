/*
 * Function: Vec3_Init
 * Address: 0x00401040
 * Signature: void __thiscall Vec3_Init(void *this,int param_1)
 *
 * Patterns: none identified. Calls: Vec3_Init. Offsets: 2, Lines: 9
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
