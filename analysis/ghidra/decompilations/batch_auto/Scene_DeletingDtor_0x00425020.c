/*
 * Function: Scene_DeletingDtor
 * Address: 0x00425020
 * Signature: void * __thiscall Scene_DeletingDtor(void *this, byte param_1)
 * Parameters: this: Scene* | param_1: byte — flags (bit 0 = free memory after destroy)
 *
 * Description:
 * Scalar deleting destructor for base Scene class. Calls Scene_dtor then conditionally frees memory based on param_1 bit 0.
 *
 * Struct offsets: N/A (destructor)
 * Cross-references: 0 calls, 1 data ref (vtable)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
