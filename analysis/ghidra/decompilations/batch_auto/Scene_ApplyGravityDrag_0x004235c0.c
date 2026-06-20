/*
 * Function: Scene_ApplyGravityDrag
 * Address: 0x004235c0
 * Signature: void __fastcall Scene_ApplyGravityDrag(int param_1)
 * Parameters: param_1: Scene* this — the scene to apply gravity drag to
 *
 * Description:
 * Applies gravity drag decay to 4+ reflection values at +0x5C58/+0x5C5C/+0x5C60/+0x5C64 by subtracting _DAT_004CF428 and clamping to minimum _DAT_004CF6A8. Called as vtable HandleRaceEnd variant — reduces mirror effects after race ends.
 *
 * Struct offsets: +0x5C58, +0x5C5C, +0x5C60, +0x5C64 (reflection decay values)
 * Cross-references: 0 calls, 1 data ref (vtable)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
