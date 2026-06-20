/*
 * Function: Matrix44_Zero
 * Address: 0x00457b10
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: int (matrix base address)
 *
 * Description:
 * Zeros a 4x4 matrix (68 bytes). Sets 12 elements to 0, then 4 diagonal elements to 1.0f at +4/+0x18/+0x2C/+0x40. 5+ refs.
 *
 * Struct offsets:
 *   +0x04/+0x18/+0x2C/+0x40 (diagonal=1.0), rest=0
 *
 * Cross-references:
 *   5+ refs from Scene_RenderWithCamera, Sawblade_Render, Gear_Render, vtables
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
