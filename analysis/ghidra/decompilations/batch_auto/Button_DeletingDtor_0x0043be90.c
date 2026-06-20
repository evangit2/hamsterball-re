/*
 * Function: Button_DeletingDtor
 * Address: 0x0043BE90
 * Signature: void * __thiscall Button_DeletingDtor(void *this, byte param_1)
 * Parameters:
 *   this: Button* — the button obstacle to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory.
 *
 * Description:
 * Scalar deleting destructor for Button objects. Standard MSVC pattern:
 *   1. Calls Button_Level_Dtor(this) — actual destructor
 *   2. If param_1 & 1, calls _free(this)
 *
 * Referenced from TWO vtables: 0x4D5578 and 0x4D5CA0 (DATA references).
 * This means Button shares its destructor across two different vtable variants.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5578 [DATA]
 *   Referenced from vtable at 0x4D5CA0 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
