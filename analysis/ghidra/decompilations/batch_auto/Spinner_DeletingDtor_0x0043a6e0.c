/*
 * Function: Spinner_DeletingDtor
 * Address: 0x0043A6E0
 * Signature: void * __thiscall Spinner_DeletingDtor(void *this, byte param_1)
 * Parameters:
 *   this: Spinner* — the spinner obstacle to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory.
 *
 * Description:
 * Scalar deleting destructor for Spinner objects. Standard MSVC pattern:
 *   1. Calls Spinner_Level_Dtor(this) — actual destructor (frees Vec3List, sub-objects)
 *   2. If param_1 & 1, calls _free(this)
 *
 * Referenced from vtable at 0x4D53F8 (DATA reference) — this is the vtable
 * set by GameLevel_ctor (mislabeled, actually a Spinner constructor).
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D53F8 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
