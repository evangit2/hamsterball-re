/*
 * Function: RumbleBoard_TickTimerWrapper
 * Address: 0x004468C0
 * Signature: void __fastcall RumbleBoard_TickTimerWrapper(int param_1)
 * Parameters:
 *   param_1: RumbleBoard* this — the board whose timer to tick
 *
 * Description:
 * Thin wrapper that calls RumbleBoard_TickTimer on the board's timer at
 * offset +0x110C, then calls NoOp (likely a vtable slot placeholder or
 * debugging hook). Referenced from 4 different vtable entries, indicating
 * it's used as a generic per-frame tick callback for multiple board types.
 *
 * Struct offsets:
 *   +0x110C: Timer object (passed to RumbleBoard_TickTimer)
 *
 * Cross-references:
 *   Referenced from vtables at 0x4D67EC, 0x4D691C, 0x4D6974, 0x4D69CC [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
