/* Ghidra Decompilation
 * Function: Vec3_Init
 * Address: 0x401040
 * Decompiled: 2026-06-19
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

