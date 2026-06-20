/*
 * Function: Vec3_Copy
 * Address: 0x00401010
 * Signature: void __thiscall Vec3_Copy(void *this, int param_1)
 *
 * Description:
 * Copies 4 floats (x,y,z,w) from source to destination. Despite the name 'Vec3', it copies
4 components (0x10 bytes = 16 bytes), suggesting this is actually a Vec4 or Matrix row copy.
Copies from param_1+4 through param_1+0x10 into this+4 through this+0x10.
NOTE: Offsets start at +4, not +0 — the first 4 bytes (vtable or type tag) are skipped.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __thiscall Vec3_Copy(void *this,int param_1)
{
  *(undefined4 *)((int)this + 4) = *(undefined4 *)(param_1 + 4);
  *(undefined4 *)((int)this + 8) = *(undefined4 *)(param_1 + 8);
  *(undefined4 *)((int)this + 0xc) = *(undefined4 *)(param_1 + 0xc);
  *(undefined4 *)((int)this + 0x10) = *(undefined4 *)(param_1 + 0x10);
  return;
}
