/*
 * Function: Glass_Level_scalar_dtor
 * Address: 0x004383F0
 * Signature: void * __thiscall Glass_Level_scalar_dtor(void *this, byte param_1)
 * Parameters:
 *   this: Glass* — the Glass level object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory after destructing.
 *
 * Description:
 * Scalar deleting destructor for Glass level objects (Glass Race / Level 12).
 * Standard MSVC pattern:
 *   1. Calls Glass_Level_Dtor(this) — the actual destructor logic
 *   2. If param_1 & 1, calls _free(this) to release memory
 *
 * Referenced from vtable at 0x4D4FF8 (DATA reference) — this vtable is set by
 * GlassStands_Ctor, making this the destructor for GlassStands objects.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D4FF8 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
