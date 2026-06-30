/*
 * Function: SimpleMenu_ctor
 * Address: 0x00448f20
 * Signature: void * __thiscall ...(void *this, int param_1)
 * Parameters:
 *   this: SimpleMenu* (vtable=0x4D6A70) | param_1: App pointer
 *
 * Description:
 * Base constructor for all menus. Inherits Gadget_ctor. Initializes AthenaList at +0x88C, stores App at +0x878, copies Font from App+0x318 to +0x87C. Sets default position (0,0,800,600). Creates 2 AthenaString objects (0x1C bytes each) at +0xCA4/+0xCA8 with pos/scale data. 7+ call sites.
 *
 * Struct offsets:
 *   +0x868 (name='Simple Menu'), +0x878 (App), +0x87C (Font from App+0x318), +0x88C (AthenaList), +0xCAC (spacing=0), +0xCB0/+0xCB4 (scroll), +0xCBC (dirty=1), +0xCA4/+0xCA8 (AthenaStrings)
 *
 * Cross-references:
 *   7+ calls from ConfirmMenu_ctor, PauseMenu_Ctor, QuitRaceMenu, PauseArenaMenu_ctor, PracticeMenu_ctor, etc.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
