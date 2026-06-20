/*
 * Function: OptionsMenu_ctor
 * Address: 0x00442CE0
 * Signature: void * __thiscall OptionsMenu_ctor(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: OptionsMenu* — being constructed (vtable = 0x4D5E30, UIList_DeletingDtor)
 *   param_1: int — parent App pointer (passed to SimpleMenu_ctor)
 *   param_2: undefined4 — stored at this+0xCDC (context/menu ID)
 *
 * Description:
 * Constructor for the Options Menu. Inherits from SimpleMenu. Steps:
 *   1. Calls SimpleMenu_ctor(this, param_1) — base class init
 *   2. Sets vtable to PTR_UIList_DeletingDtor (0x4D5E30)
 *   3. Initializes 5 Transform objects at +0xCF0, +0xD28, +0xD60, +0xD98, +0xDD0
 *   4. Stores param_2 at +0xCDC (menu context)
 *   5. Sets title "OPTIONS" at +0x888, "Options Menu" at +0x868
 *   6. Sets +0xE09 = 0 (fullscreen flag), +0xCAC = 0x15E (350, item spacing?)
 *   7. Copies display params from App→+0x878→+0x320 (+800) to +0x87C
 *   8. Sets +0x884 = 1 (menu visible)
 *   9. Adds UIList items:
 *      - "RESOLUTION: 1024 X 768" → "REZ"
 *      - "FULLSCREEN: YES" → "FS"
 *      - "COLOR QUALITY: ..." → "CM"
 *      - "SAFE MODE: ..." → "SM"
 *      - "PAUSE W/RIGHT BUTTON: ..." → "PWRB"
 *      - Sound Volume → "SV"
 *      - Music Volume → "MV"
 *      - Texture Quality → "TQ"
 *      - Master Setting → "MS"
 *
 * Each item uses Matrix_Scale4x4(1.0, 1.0, 1.0, 1.0) for identity scaling.
 *
 * Struct offsets:
 *   +0x868: menu name string ("Options Menu")
 *   +0x878: App pointer
 *   +0x888: title string ("OPTIONS")
 *   +0x87C: display config (from App)
 *   +0x884: visible flag (1)
 *   +0xCAC: item spacing (0x15E = 350)
 *   +0xCDC: menu context (param_2)
 *   +0xCF0/+0xD28/+0xD60/+0xD98/+0xDD0: Transform objects (5 × ~56 bytes)
 *   +0xD10: Sound Volume
 *   +0xD48: Music Volume
 *   +0xD80: Texture Quality
 *   +0xDF0: Master Setting
 *   +0xE09: fullscreen flag (0)
 *
 * Cross-references:
 *   Called from PauseMenu_HandleButtonClick (0x4316DB) — UNCONDITIONAL_CALL
 *   Called from TourneyMenu_HandleButton (0x45065D) — UNCONDITIONAL_CALL
 *   Called from 0x430551, 0x4306FA, 0x430B37 — UNCONDITIONAL_CALL
 *   Called from 0x42E6B0 — UNCONDITIONAL_CALL
 *   (6 total call sites)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
