/*
 * Function: UITimer_Ctor
 * Address: 0x00448ac0
 * Signature: undefined4 * __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: UITimer* this — being constructed
 *
 * Description:
 * Constructor for UITimer (embedded in Ball). Calls Timer_Init, sets vtable to 0x4D6A20. Initializes ~60 fields: 5 float pairs set to 1.0f (0x3F800000) at offsets +0x17/+0x1C/+0x21/+0x26/+0x27/+0x2C/+0x31/+0x36, rest zeroed. 1 call from Ball_ctor2.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4D6A20), +0x11 through +0x3A (various timer fields), 5 float pairs at 1.0f
 *
 * Cross-references:
 *   1 call from Ball_ctor2 (0x403A0D)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
