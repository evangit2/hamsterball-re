/*
 * Function: QuitDialog_ctor
 * Address: 0x00443E30
 * Signature: void * __thiscall QuitDialog_ctor(void *this, int param_1)
 * Parameters:
 *   this: QuitDialog* — being constructed (vtable = 0x4D62C0, GameObject2)
 *   param_1: int — parent App pointer (stored at +0x878)
 *
 * Description:
 * Base constructor for all Quit dialog variants. Inherits from Gadget_ctor.
 * Sets up two AthenaString objects at +0x87C and +0x898, appends both to
 * AthenaList at +0x44C. Stores position floats at +4/+8/+0xC/+0x10
 * (112.0, 150.0, 500.0, 300.0). Computes relative positions from _DAT_004D62AC
 * and _DAT_004D62A8. Sets +0x8B4 = 0. Registers dialog in App→+0x184→+0x420.
 *
 * Struct offsets:
 *   +0x04/+0x08/+0x0C/+0x10: position (112.0, 150.0, 500.0, 300.0)
 *   +0x44C: AthenaList (contains dialog strings)
 *   +0x868: "Quit Dialog" (name)
 *   +0x878: App pointer
 *   +0x87C: AthenaString #1
 *   +0x898: AthenaString #2
 *   +0x880/+0x884/+0x888/+0x88C: secondary rect (144.0, 182.0, 96.0, 56.0)
 *   +0x89C/+0x8A0: computed relative positions
 *   +0x8A4/+0x8A8: sizes (96.0, 56.0)
 *   +0x8B4: flag (0)
 *
 * Cross-references:
 *   Called from TourneyContinueDialog_Ctor (0x445E83) — UNCONDITIONAL_CALL
 *   Called from QuitToDesktopDialog_Ctor (0x4447B3) — UNCONDITIONAL_CALL
 *   Called from QuitAbortDialog_Ctor (0x445343) — UNCONDITIONAL_CALL
 *   Called from 0x43060D — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
