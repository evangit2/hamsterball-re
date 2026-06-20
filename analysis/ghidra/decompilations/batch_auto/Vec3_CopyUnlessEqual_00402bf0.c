/*
 * Function: Vec3_CopyUnlessEqual
 * Address: 0x00402bf0
 * Signature: void __thiscall Vec3_CopyUnlessEqual(void *this,undefined4 *param_1)
 *
 * Patterns: none identified. Calls: Vec3_CopyUnlessEqual. Offsets: 0, Lines: 9
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_CopyUnlessEqual(void *this,undefined4 *param_1)

{
  if (this != param_1) {
    *(undefined4 *)this = *param_1;
    *(undefined4 *)((int)this + 4) = param_1[1];
    *(undefined4 *)((int)this + 8) = param_1[2];
  }
  return;
}
