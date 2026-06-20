/*
 * Function: AthenaList_GetAt
 * Address: 0x0040a020
 *
 * Description:
 *
Gets an element at a specific index from an AthenaList. Bounds-checks the index
against the count at this+4, then returns the element from the array at this+0x40C.
Returns 0 if index is out of bounds.

AthenaList layout: +0x04=count, +0x40C=element array pointer.
Cross-refs: 2 calls (collision/spatial functions at 0x439F26, 0x439FD0).

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

