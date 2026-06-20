/*
 * Function: ScoreDisplay_scalar_dtor
 * Address: 0x00447370
 * Signature: void * __thiscall ScoreDisplay_scalar_dtor(void *this, byte param_1)
 * Parameters:
 *   this: ScoreDisplay* — the score display to destroy
 *   param_1: byte — scalar deleting flag (bit 0 = free memory)
 *
 * Description:
 * Scalar deleting destructor for ScoreDisplay. Calls ScoreDisplay_dtor,
 * then if param_1 & 1, calls _free(this) to release the object's memory.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D67E8 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
