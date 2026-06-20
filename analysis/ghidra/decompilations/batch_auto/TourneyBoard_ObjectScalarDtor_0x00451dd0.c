/*
 * Function: TourneyBoard_ObjectScalarDtor
 * Address: 0x00451dd0
 * Signature: void * __thiscall ...(void *this, byte param_1)
 * Parameters:
 *   this: TourneyBoard* | param_1: scalar delete flag
 *
 * Description:
 * Scalar deleting dtor. Calls TourneyBoard_ObjectDtorInner then _free if bit 0. 1 DATA ref.
 *
 * Struct offsets:
 *   N/A (dispatcher)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D8628
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
