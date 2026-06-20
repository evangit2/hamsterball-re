/*
 * Function: Gear_Render
 * Address: 0x0043A270
 * Signature: void __fastcall Gear_Render(int param_1)
 * Parameters:
 *   param_1: Gear* this — the gear obstacle to render
 *
 * Description:
 * Render function for Gear obstacles. Complex function that:
 *   1. Initializes a Timer and calls through vtable+0x18 (render setup)
 *   2. Sets Gfx_ScaleX using rotation angle at +0x10E0 + a global constant (_DAT_004D5C80)
 *   3. Computes Y-rotation offset using Wave_Sin (sine wave lookup) with the
 *      Y-rotation value at +0x10E4
 *   4. Calls Gfx_SetPosition with computed position (uses +0x10D4 X, adjusted Y
 *      from base position + timer + sine offset, +0x10DC Z)
 *   5. Renders the gear mesh via vtable chain: App→Scene+0x10D0→+0x878→+0x174→+0x154→+200
 *      (this is the MeshWorld render call)
 *   6. Renders attached sub-objects via Scene→+0x4BB0→+0x1C
 *   7. Renders mesh again (double render for shadow/reflection?)
 *   8. Zeros a 4x4 matrix and repeats with different scale/position
 *
 * The gear animation uses Wave_Sin for smooth oscillation — the gear bobs up
 * and down while rotating, creating a dynamic visual effect.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer (chain to App→MeshWorld)
 *   +0x10D4: position X
 *   +0x10D8: position Y (base, adjusted with timer and sine)
 *   +0x10DC: position Z
 *   +0x10E0: rotation angle (X scale)
 *   +0x10E4: Y rotation (used with Wave_Sin)
 *   +0x10F8: timer (affects Y offset)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5300 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
