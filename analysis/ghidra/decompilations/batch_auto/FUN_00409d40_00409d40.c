/*
 * Function: FUN_00409d40
 * Address: 0x00409D40
 * Signature: void __fastcall FUN_00409d40(int param_1)
 *
 * Description:
 * Small initializer for an unknown object. Sets three fields:
 *   +0x04 = 0  (counter or state, cleared)
 *   +0x08 = 0  (byte flag, cleared)
 *   +0x0C = 10 (integer, likely a default timer/count value)
 *
 * The object appears to be small (accessed via param_1 directly, offsets 4-0xC).
 * Possibly a timer or counter object used by the UI system.
 *
 * Cross-references:
 *   - Referenced from vtable at 0x4CF584 (data ref) — likely a BaseObject vtable
 *     entry, meaning this is called as a virtual init function
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall FUN_00409d40(int param_1)

{
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined1 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 10;
  return;
}
