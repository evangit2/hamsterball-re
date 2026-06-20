/*
 * Function: GameObject_sub2_dtor
 * Address: 0x00402A50
 * Signature: void * __thiscall GameObject_sub2_dtor(void *this, byte param_1)
 *
 * Description:
 * Scalar deleting destructor for the GameObject_sub2 class (a lightweight
 * sub-class of GameObject). Follows the standard MSVC pattern:
 *   1. Calls GameObject_dtor(this) — the full base class destructor
 *      (which cleans up timers, matrices, Vec3Lists, child objects, etc.)
 *   2. If param_1 & 1 (deleting flag): calls _free(this) to release heap memory
 *   3. Returns this
 *
 * This is simpler than Ball_dtor2 because GameObject_sub2 doesn't add any
 * class-specific cleanup beyond the base GameObject cleanup.
 *
 * Cross-references:
 *   - Referenced from vtable at 0x4CF314 (the GameObject_sub2 vtable, also
 *     used as the base class vtable set by GameObject_dtor during destruction)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall GameObject_sub2_dtor(void *this,byte param_1)

{
  GameObject_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
