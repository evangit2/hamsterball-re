/*
 * Function: FUN_00409dc0
 * Address: 0x00409DC0
 * Signature: void __fastcall FUN_00409dc0(int param_1)
 *
 * Description:
 * Initializes a large object (likely a game/application state object) by
 * setting three fields:
 *   +0x4358 = 0  (counter/state, cleared)
 *   +0x0874 = 0  (byte flag, cleared)
 *   +0x4368 = 1  (byte flag, set — likely "active" or "initialized")
 *
 * The large offsets (0x4358, 0x4368) suggest this operates on the App or
 * a similar top-level game state object that is several KB in size.
 *
 * Cross-references:
 *   - Called from QuitToDesktop_Execute (0x446AE0) — this initialization is
 *     performed when the player chooses to quit the game to desktop, likely
 *     resetting game state for the shutdown sequence
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall FUN_00409dc0(int param_1)

{
  *(undefined4 *)(param_1 + 0x4358) = 0;
  *(undefined1 *)(param_1 + 0x874) = 0;
  *(undefined1 *)(param_1 + 0x4368) = 1;
  return;
}
