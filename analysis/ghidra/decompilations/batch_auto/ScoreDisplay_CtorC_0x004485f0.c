/*
 * Function: ScoreDisplay_CtorC
 * Address: 0x004485f0
 * Signature: void * __thiscall ...(void *this, int param_1, byte *param_2, undefined4 param_3)
 * Parameters:
 *   this: ScoreDisplay* (vtable=0x4D69C8) | param_1: App | param_2: byte* (serial/affiliate key) | param_3: undefined4 (stored at +0x115C)
 *
 * Description:
 * Calls RegisterDialog_ctor, overrides vtable to PTR_ScoreDisplay_DeletingDtor (0x4D69C8). Stores param_3 at +0x115C. 1 call from Scene_Update.
 *
 * Struct offsets:
 *   +0x115C (param_3)
 *
 * Cross-references:
 *   1 call from Scene_Update (0x419CA4)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
