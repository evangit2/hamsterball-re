/*
 * Function: KeyRemapMenu_Ctor
 * Address: 0x004431E0
 * Signature: void * __thiscall KeyRemapMenu_Ctor(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: KeyRemapMenu* — being constructed (vtable = 0x4D5F50, KeyRemapMenu_DeletingDtor)
 *   param_1: int — parent App pointer (passed to SimpleMenu_ctor)
 *   param_2: undefined4 — stored at this+0xCDC (context/menu ID)
 *
 * Description:
 * Constructor for the Key Remap Menu. Inherits from SimpleMenu. Steps:
 *   1. Calls SimpleMenu_ctor(this, param_1) — base class init
 *   2. Stores param_2 at +0xCDC (menu context)
 *   3. Sets vtable to PTR_KeyRemapMenu_DeletingDtor (0x4D5F50)
 *   4. Sets title "Remap Keyboard" at +0x888
 *   5. Sets +0xCAC = 0x15E (350, item spacing)
 *   6. Sets +0x868 = "Remap Menu" (menu name)
 *   7. Copies display params from App→+0x878→+0x320 (offset 800) to +0x87C
 *   8. Sets +0x884 = 1 (menu visible)
 *   9. Adds 6 UIList items:
 *      - "Key Up:" → "UP"
 *      - "Key Down:" → "DOWN"
 *      - "Key Left:" → "LEFT"
 *      - "Key Right:" → "RIGHT"
 *      - "Key Action1:" → "ACTION1"
 *      - "Key Action2:" → "ACTION2"
 *   10. Calls KeyRemapMenu_UpdateKeyLabels to populate current key bindings
 *
 * Each item uses Matrix_Scale4x4(1.0, 1.0, 1.0, 1.0) for identity scaling.
 *
 * Struct offsets:
 *   +0x868: menu name ("Remap Menu")
 *   +0x878: App pointer
 *   +0x888: title ("Remap Keyboard")
 *   +0x87C: display config (from App)
 *   +0x884: visible (1)
 *   +0xCAC: item spacing (0x15E = 350)
 *   +0xCDC: menu context (param_2)
 *
 * Cross-references:
 *   Called from 0x44394A — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
