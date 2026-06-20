/*
 * Function: QuitAbortDialog_Ctor
 * Address: 0x00445320
 * Signature: void * __thiscall QuitAbortDialog_Ctor(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: QuitAbortDialog* — being constructed (vtable = 0x4D64C8)
 *   param_1: int — parent App pointer
 *   param_2: undefined4 — stored at +0x8B8 (dialog context)
 *
 * Description:
 * Constructor for "Quit Abort" confirmation dialog. Calls QuitDialog_ctor,
 * then overrides vtable to PTR_GameObject2_scalar_dtor (0x4D64C8).
 * Similar to QuitToDesktopDialog_Ctor but with different vtable.
 *
 * Cross-references:
 *   Called from TourneyMenu_HandleButton (0x450485) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
