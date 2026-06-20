/*
 * Function: AthenaList_GetAt
 * Address: 0x0040a020
 * Signature: undefined4 __thiscall AthenaList_GetAt(void *this,int param_1)
 *
 * Patterns: none identified. Calls: AthenaList_GetAt. Offsets: 1, Lines: 7
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

undefined4 __thiscall AthenaList_GetAt(void *this,int param_1)

{
  if ((-1 < param_1) && (param_1 < *(int *)((int)this + 4))) {
    return *(undefined4 *)(*(int *)((int)this + 0x40c) + param_1 * 4);
  }
  return 0;
}
