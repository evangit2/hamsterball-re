/*
 * Function: AthenaList_SetIndex
 * Address: 0x00402BC0
 * Signature: undefined4 __thiscall AthenaList_SetIndex(void *this, int param_1)
 *
 * Description:
 * Sets the active iteration index in an AthenaList (the engine's custom linked list).
 *
 * Logic:
 *   1. Clears the bit flag at this + param_1*4 + 8 (sets to 0 — marks element
 *      as "not current" at the given index)
 *   2. If the list has elements (count at this+4 > 0):
 *      a. Gets the first element's value from the array at this+0x40C
 *      b. Sets the bit flag at this + param_1*4 + 8 to 1 (marks as "current")
 *      c. Returns the first element's value
 *   3. If list is empty: returns 0
 *
 * AthenaList layout (deduced):
 *   +0x00: vtable pointer
 *   +0x04: element count (int)
 *   +0x08: Bitfield array — one int per index slot, used as "current" flags
 *   +0x40C: Pointer to element array (or first element pointer)
 *
 * This function is used during iteration to select which "view" of the list
 * is active — multiple indices can be used simultaneously for nested iteration.
 *
 * Cross-references (8 call sites):
 *   - Sound_PlayChannel — selecting audio channels
 *   - Scene_RenderAllObjects — iterating renderable objects
 *   - FollowBall_Update (3 calls) — AI ball iteration
 *   - BoardLevel3_ctor — Dizzy Race level construction
 *   - RumbleObject_Update — rumble object iteration
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
