/*
 * Function: Tipper_Render
 * Address: 0x0043A5E0
 * Signature: void __fastcall Tipper_Render(int param_1)
 * Parameters:
 *   param_1: Tipper* this — the TipperVisual object to render
 *
 * Description:
 * Render function for TipperVisual objects. The TipperVisual renders breakable
 * objects (like glass) with a tipping animation.
 *
 * Steps:
 *   1. Initializes a Timer
 *   2. Computes Y-scale using Wave_Sin: sin(+0x10E4) × +0x10E0 × _DAT_004CF470
 *      — this creates a oscillating scale effect (the glass tips back and forth)
 *   3. Copies position from +0x10D4/+0x10D8/+0x10DC to local variables
 *   4. Calls Timer vtable[2] (render) with position
 *   5. Calls through Scene→+0x878→+0x270→vtable[7] to render the mesh
 *   6. Cleans up Timer
 *
 * The tipping animation uses a sine wave to smoothly oscillate the Y-scale,
 * creating the visual effect of glass panels rocking back and forth before
 * falling away.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer (chain to App→MeshWorld)
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0: tilt amplitude (scale factor)
 *   +0x10E4: tilt angle (used with Wave_Sin for oscillation)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5378 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
