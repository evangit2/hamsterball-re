/*
 * Function: Gfx_LoadMatrixFromStack
 * Address: 0x00457ab0
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: undefined4* (destination matrix, 16 floats)
 *
 * Description:
 * Copies 16 floats from stack (offset 0x08) to param_1. Used in timer array dtors. 2 calls.
 *
 * Struct offsets:
 *   +0x00 (16-float matrix filled from stack)
 *
 * Cross-references:
 *   2 calls from TimerArray_FullDtor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
