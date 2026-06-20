/*
 * Function: Cascade_Level_scalar_dtor
 * Address: 0x00438830
 * Signature: void * __thiscall Cascade_Level_scalar_dtor(void *this, byte param_1)
 * Parameters:
 *   this: Cascade_Level* — the Cascade level object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory after destructing.
 *
 * Description:
 * Scalar deleting destructor for Cascade level objects (Beginner Race / Level 2).
 * Standard MSVC pattern:
 *   1. Calls Cascade_Level_Dtor(this) — the actual destructor logic
 *   2. If param_1 & 1, calls _free(this) to release memory
 *
 * Referenced from vtable at 0x4D50C0 (DATA reference) — this vtable is set by
 * CascadeStands_Ctor, making this the destructor for CascadeStands objects.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D50C0 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
