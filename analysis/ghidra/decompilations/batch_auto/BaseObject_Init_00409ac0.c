/*
 * Function: BaseObject_Init
 * Address: 0x00409AC0
 * Signature: void __fastcall BaseObject_Init(undefined4 *param_1)
 *
 * Description:
 * Minimal constructor for BaseObject — the root base class in the engine's
 * class hierarchy. Sets only the vtable pointer to PTR_LAB_004cf584.
 *
 * This is the simplest constructor in the engine — it just sets the vtable.
 * All actual initialization is done by derived class constructors.
 *
 * Cross-references (54 call sites):
 *   - Called from virtually every object constructor in the engine
 *   - GameObject2_dtor (cleanup during destruction)
 *   - Unwind handlers (exception cleanup)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall BaseObject_Init(undefined4 *param_1)

{
  *param_1 = &PTR_LAB_004cf584;
  return;
}
