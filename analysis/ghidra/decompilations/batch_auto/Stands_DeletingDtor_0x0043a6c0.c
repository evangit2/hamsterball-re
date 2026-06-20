/*
 * Function: Stands_DeletingDtor
 * Address: 0x0043A6C0
 * Signature: void * __thiscall Stands_DeletingDtor(void *this, byte param_1)
 * Parameters:
 *   this: Stands* — the stands object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory.
 *
 * Description:
 * Scalar deleting destructor for Stands objects. Standard MSVC pattern:
 *   1. Calls Stands_Dtor(this) — actual destructor (sets vtable, calls Level_Cleanup)
 *   2. If param_1 & 1, calls _free(this) to release memory
 *
 * Referenced from vtable at 0x4D5390 (DATA reference) — this is the vtable
 * set by Stands_CtorWithCollision.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5390 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
