/*
 * Function: Impossible_Level_scalar_dtor
 * Address: 0x00438730
 * Signature: void * __thiscall Impossible_Level_scalar_dtor(void *this, byte param_1)
 * Parameters:
 *   this: Impossible_Level* — the Impossible level object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory after destructing.
 *
 * Description:
 * Scalar deleting destructor for Impossible level objects (Impossible Race / Level 15).
 * Also used by Glass_Level_ctor (shares vtable 0x4D5060).
 * Standard MSVC pattern:
 *   1. Calls Impossible_Level_Dtor(this) — the actual destructor logic
 *   2. If param_1 & 1, calls _free(this) to release memory
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5060 [DATA]
 *   This vtable is shared by Glass_Level_ctor (0x4384A0) and Impossible level objects.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
