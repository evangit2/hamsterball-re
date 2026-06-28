/*
 * Function: RaceGoalReached_Ctor
 * Address: 0x0044c880
 * Signature: void * __thiscall ...(void *this, int param_1, undefined4 param_2)
 * Parameters:
 *   this: RaceGoalReached* (vtable=0x4D6CF0) | param_1: parent | param_2: undefined4 (stored at +8)
 *
 * Description:
 * Constructor for 'GOAL REACHED!' display. Stores parent at +4, param_2 at +8, App at +0xC. Increments active count at parent+0x8B4. Sets max frames=100 (+0x14). Vtable→0x4D6CF0. Initializes SceneObject_EmptyListCtor at +0x90, ToggleTimer_Init at +0x4C4. Sets 'GOAL REACHED!' text at +0x24. Width=800 (+0x20/+0x4C0). 1 call.
 *
 * Struct offsets:
 *   +0x04 (parent), +0x08 (param_2), +0x0C (App), +0x10 (frame=0), +0x14 (max=100), +0x18 (flag=0), +0x19 (active=1), +0x20 (width=800), +0x24 ('GOAL REACHED!'), +0x90 (SceneObject list), +0x4AC-0x4BC (zeros), +0x4C0 (width=800), +0x4C4 (timer)
 *
 * Cross-references:
 *   1 call from 0x409F51
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
