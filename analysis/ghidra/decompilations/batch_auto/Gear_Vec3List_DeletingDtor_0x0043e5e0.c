/*
 * Function: Gear_Vec3List_DeletingDtor
 * Address: 0x0043E5E0
 * Signature: void * __thiscall Gear_Vec3List_DeletingDtor(void *this, byte param_1)
 * Parameters:
 *   this: Gear_Vec3List* — the gear Vec3List object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory.
 *
 * Description:
 * Scalar deleting destructor for Gear_Vec3List objects. Standard MSVC pattern:
 *   1. Calls Gear_Vec3List_Level_Dtor(this) — actual destructor
 *   2. If param_1 & 1, calls _free(this)
 *
 * Referenced from vtable at 0x4D5AD0 (DATA reference).
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5AD0 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
