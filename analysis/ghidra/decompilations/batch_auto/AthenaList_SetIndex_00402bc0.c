/*
 * Function: AthenaList_SetIndex
 * Address: 0x00402bc0
 * Signature: undefined4 __thiscall AthenaList_SetIndex(void *this,int param_1)
 *
 * Patterns: none identified. Calls: AthenaList_SetIndex. Offsets: 1, Lines: 12
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

undefined4 __thiscall AthenaList_SetIndex(void *this,int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 0;
  *(undefined4 *)((int)this + param_1 * 4 + 8) = 0;
  if (0 < *(int *)((int)this + 4)) {
    uVar1 = **(undefined4 **)((int)this + 0x40c);
    *(undefined4 *)((int)this + param_1 * 4 + 8) = 1;
  }
  return uVar1;
}
