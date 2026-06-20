/*
 * Function: Timer_dtor
 * Address: 0x00401640
 * Signature: void * __thiscall Timer_dtor(void *this, byte param_1)
 *
 * Description:
 * Scalar deleting destructor for the Timer class.
 * Follows the standard MSVC pattern:
 *   1. Calls Timer_Cleanup(this) — which resets the vtable pointer to
 *      PTR_Timer_dtor_004cf338 (the Timer base class vtable). This ensures
 *      that if the destructor is called again during stack unwinding, it
 *      dispatches to the base Timer_dtor rather than a derived class's dtor.
 *   2. If param_1 & 1 (the "deleting" flag), calls _free(this) to release
 *      the heap memory.
 *   3. Returns this (for chaining / operator delete).
 *
 * The Timer class is a small object — Timer_Cleanup only sets the vtable,
 * meaning Timer has no owned heap resources to clean up (it likely just
 * holds timing state as inline fields). The actual timer logic (start/stop/
 * elapsed) is handled elsewhere.
 *
 * Cross-references:
 *   - Referenced from vtable at 0x4CF338 (Timer vtable)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall Timer_dtor(void *this,byte param_1)

{
  Timer_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
