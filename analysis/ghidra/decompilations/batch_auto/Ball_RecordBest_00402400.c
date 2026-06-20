/*
 * Function: Ball_RecordBest
 * Address: 0x00402400
 * Signature: void __thiscall Ball_RecordBest(void *this,int param_1)
 *
 * Patterns: ball. Calls: Ball_RecordBest. Offsets: 2, Lines: 10
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_RecordBest(void *this,int param_1)

{
  *(undefined4 *)((int)this + 0x2ec) = 0;
  if (*(int *)((int)this + 0x2f4) < param_1) {
    *(int *)((int)this + 0x2f4) = param_1;
    return;
  }
  *(int *)((int)this + 0x2f4) = *(int *)((int)this + 0x2f4);
  return;
}
