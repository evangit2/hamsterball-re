/*
 * Function: Gear_DeletingDtor
 * Address: 0x0043A250
 * Signature: void * __thiscall Gear_DeletingDtor(void *this, byte param_1)
 * Parameters:
 *   this: Gear* — the gear obstacle to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory after destructing.
 *
 * Description:
 * Scalar deleting destructor for Gear objects. Standard MSVC pattern:
 *   1. Calls Gear_Level_Dtor(this) — the actual destructor logic
 *   2. If param_1 & 1, calls _free(this) to release memory
 *
 * Referenced from vtable at 0x4D52B8 (DATA reference) — this is the vtable
 * set by Gear_Level_ctor.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D52B8 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
