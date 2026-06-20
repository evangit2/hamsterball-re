/*
 * Function: SaveTourneyDialog_ctor
 * Address: 0x0044fd60
 * Signature: void * __thiscall ...(void *this, int param_1, char *param_2, byte *param_3, undefined4 param_4, int param_5)
 * Parameters:
 *   this: SaveTourneyDialog* (vtable=0x4D8358) | param_1: App | param_2: char* title | param_3: byte* body text | param_4: undefined4 (stored at +0x8A8) | param_5: int (duration)
 *
 * Description:
 * Constructor for SaveTourneyDialog. Calls OkayDialog_ctor (parent), overrides vtable to 0x4D8358. Sets name 'SaveTourneyDialog' at +0x868. Stores param_4 at +0x8A8. 1 call.
 *
 * Struct offsets:
 *   +0x868 ('SaveTourneyDialog'), +0x8A8 (param_4)
 *
 * Cross-references:
 *   1 call from TourneyMenu_HandleButton (0x4505B7)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
