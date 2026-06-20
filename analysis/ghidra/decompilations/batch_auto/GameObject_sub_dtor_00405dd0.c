/*
 * Function: GameObject_sub_dtor
 * Address: 0x00405DD0
 * Signature: void * __thiscall GameObject_sub_dtor(void *this, byte param_1)
 *
 * Description:
 * Scalar deleting destructor for the GameObject_sub class. Simply delegates
 * to the base GameObject_dtor for cleanup, then conditionally frees memory.
 * No sub-class specific cleanup needed (the sub-class only sets a few fields
 * that are cleaned up by the base destructor).
 *
 * Cross-references:
 *   - Referenced from vtables at 0x4CF314, 0x4CF49C, 0x4CF564 (data references)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall GameObject_sub_dtor(void *this,byte param_1)

{
  GameObject_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
