/*
 * Function: Ball_SetVec3AtOffset
 * Address: 0x00402a20
 * Signature: void __thiscall Ball_SetVec3AtOffset(void *this,undefined4 *param_1)
 *
 * Patterns: ball. Calls: Ball_SetVec3AtOffset. Offsets: 3, Lines: 9
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_SetVec3AtOffset(void *this,undefined4 *param_1)

{
  if ((undefined4 *)((int)this + 0xca4) != param_1) {
    *(undefined4 *)((int)this + 0xca4) = *param_1;
    *(undefined4 *)((int)this + 0xca8) = param_1[1];
    *(undefined4 *)((int)this + 0xcac) = param_1[2];
  }
  return;
}
