/*
 * Function: Ball_dtor2
 * Address: 0x00401CC0
 * Signature: void __fastcall Ball_dtor2(undefined4 *param_1)
 *
 * Description:
 * Secondary destructor for the Ball class. Called from Ball_dtor (0x4027F0).
 * This is a "deleting destructor" variant that:
 *   1. Sets the vtable to PTR_Ball_dtor_004cf3a0 (Ball class vtable at 0x4CF3A0)
 *   2. Resets the matrix at offset +0x321*4 = +0xC84 to identity
 *      (this offset corresponds to the Ball's local transform matrix)
 *   3. Calls GameObject_dtor(param_1) to perform the full base class cleanup
 *
 * Uses SEH frame for exception safety during destruction.
 *
 * The distinction between Ball_dtor and Ball_dtor2:
 *   - Ball_dtor (0x4027F0) is the primary scalar deleting destructor
 *   - Ball_dtor2 is called from Ball_dtor to perform the class-specific
 *     cleanup before the base GameObject_dtor runs
 *
 * Cross-references:
 *   - Called from Ball_dtor (0x4027F0) — the primary deleting destructor
 *
 * Struct offsets:
 *   ball+0x00: vtable pointer (set to 0x4CF3A0)
 *   ball+0xC84: Local transform matrix (reset to identity)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_dtor2(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004c9208;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_Ball_dtor_004cf3a0;
  local_4 = 0;
  Matrix_Identity(param_1 + 0x321);
  local_4 = 0xffffffff;
  GameObject_dtor(param_1);
  ExceptionList = local_c;
  return;
}
