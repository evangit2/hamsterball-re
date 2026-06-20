/*
 * Function: ConfirmMenu_Ctor
 * Address: 0x0044c780
 * Signature: void __thiscall ...(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: ConfirmMenu* (vtable=0x4D6CB8) | param_1: parent (Board/App) | param_2: undefined4 (stored at +8)
 *
 * Description:
 * Constructor for ConfirmMenu. Stores parent at +4, param_2 at +8, App at +0xC (from parent+0x878). Increments active count at parent+0x8B4. Sets max frames=500 (+0x14). Width=800 (+0x1C). Vtable→0x4D6CB8. 2 calls.
 *
 * Struct offsets:
 *   +0x04 (parent), +0x08 (param_2), +0x0C (App), +0x10 (frame=0), +0x14 (max=500), +0x18 (flag=0), +0x19 (active=1), +0x1C (width=800), +0x20 (flag=0)
 *
 * Cross-references:
 *   2 calls from 0x409ED3, 0x409FE2
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
