/*
 * Function: UITimer_DeletingDtor
 * Address: 0x00448bc0
 * Signature: void * __thiscall ...(void *this, byte param_1)
 * Parameters:
 *   this: UITimer* | param_1: scalar delete flag
 *
 * Description:
 * Scalar deleting dtor for UITimer. Calls UITimer_dtor then _free if bit 0. 1 DATA ref from vtable.
 *
 * Struct offsets:
 *   N/A (dispatcher)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D6A20
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
