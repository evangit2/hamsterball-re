/*
 * Function: QuitToDesktopDialog_Ctor
 * Address: 0x00444790
 * Signature: void * __thiscall QuitToDesktopDialog_Ctor(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: QuitToDesktopDialog* — being constructed (vtable = 0x4D6368)
 *   param_1: int — parent App pointer
 *   param_2: undefined4 — stored at +0x8B8 (dialog context/parent menu)
 *
 * Description:
 * Constructor for "Quit To Desktop" confirmation dialog. Calls QuitDialog_ctor
 * for base init, then overrides vtable to PTR_QuitToDesktop_scalar_dtor (0x4D6368).
 * Adjusts position and sizes. Stores param_2 at +0x8B8.
 *
 * Struct offsets:
 *   +0x8B8: dialog context (param_2)
 *   Inherits all QuitDialog_ctor offsets
 *
 * Cross-references:
 *   Called from 0x42E648 — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
