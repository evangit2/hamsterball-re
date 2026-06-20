/*
 * Function: Tipper_DeletingDtor
 * Address: 0x0043A5C0
 * Signature: void * __thiscall Tipper_DeletingDtor(void *this, byte param_1)
 * Parameters:
 *   this: Tipper* — the Tipper visual object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 set, free memory.
 *
 * Description:
 * Scalar deleting destructor for Tipper (TipperVisual) objects. Standard MSVC pattern:
 *   1. Calls Tipper_Level_Dtor(this) — actual destructor
 *   2. If param_1 & 1, calls _free(this)
 *
 * TipperVisual is the visual component used by glass stands and other breakable
 * objects to play tipping/falling animations when they break.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5330 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
