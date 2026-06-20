/*
 * Function: IndexList_Dtor
 * Address: 0x00453740
 * Signature: void * __thiscall ...(void *this, byte param_1)
 * Parameters:
 *   this: IndexList* | param_1: scalar delete flag
 *
 * Description:
 * Scalar deleting dtor for IndexList. Sets vtable 0x4D875C. If +0x40C (heap array) non-NULL: _free, set to 0. Then _free(this) if bit 0. 1 DATA ref.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4D875C), +0x40C (heap array→freed)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D875C
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
