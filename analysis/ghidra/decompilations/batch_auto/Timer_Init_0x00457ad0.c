/*
 * Function: Timer_Init
 * Address: 0x00457ad0
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Timer* this — being initialized
 *
 * Description:
 * Initializes Timer. Sets vtable 0x4CF338. Zeros 12 fields (+0x08 through +0x3C). Sets 4 floats to 1.0f at +0x04/+0x18/+0x18/+0x2C/+0x40. 4+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4CF338), +0x04/+0x18/+0x2C/+0x40 (1.0f), +0x08-0x3C (zeros)
 *
 * Cross-references:
 *   4+ calls from Graphics_SetViewport, App_Run, Ball_Render, Ball_ctor2
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
