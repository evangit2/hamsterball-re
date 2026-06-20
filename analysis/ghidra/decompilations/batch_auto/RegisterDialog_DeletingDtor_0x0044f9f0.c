/*
 * Function: RegisterDialog_DeletingDtor
 * Address: 0x0044f9f0
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: RegisterDialog* this
 *
 * Description:
 * Deleting dtor for RegisterDialog. Initializes Timer, multiplies timer value (+0x1C) by _DAT_004D6E40, calls vtable+0x18, Gfx_SetPosition, dispatches vtable+0x1C on parent+0x5A4, Timer_Cleanup. 1 DATA ref.
 *
 * Struct offsets:
 *   +0x04 (parent→+0x5A4), +0x08 (Gfx), +0x0C/+0x10 (position), +0x14 (offset), +0x1C (timer×_DAT_004D6E40)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D6E5C
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
