/*
 * Function: RaceResultsMenu_ctor
 * Address: 0x0044cb10
 * Signature: void __thiscall ...(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: RaceResultsMenu* (vtable=0x4D6CFC) | param_1: parent | param_2: undefined4 (stored at +8)
 *
 * Description:
 * Constructor for RaceResultsMenu. Stores parent at +4, param_2 at +8, App at +0xC. Increments active count at parent+0x8B4. Sets max frames=500 (+0x14), width=800 (+0x1C). Vtable→0x4D6CFC. Sets scale=1.0f at +0x20. 1 call from RumbleBoard_Update.
 *
 * Struct offsets:
 *   +0x04 (parent), +0x08 (param_2), +0x0C (App), +0x10 (frame=0), +0x14 (max=500), +0x18 (flag=0), +0x19 (active=1), +0x1C (width=800), +0x20 (scale=1.0f), +0x24 (flag=0), +0x25 (flag=0)
 *
 * Cross-references:
 *   1 call from RumbleBoard_Update (0x422240)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
