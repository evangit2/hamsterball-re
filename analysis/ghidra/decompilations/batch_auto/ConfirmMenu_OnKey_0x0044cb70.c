/*
 * Function: ConfirmMenu_OnKey
 * Address: 0x0044cb70
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: ConfirmMenu* this
 *
 * Description:
 * Key handler for ConfirmMenu. Sets vtable to 0x4D6CFC. Reads index from param_1[2]+0x14, accesses array at param_1[3]+0xB38+index*4. Adds 4 to that int. Resets vtable to 0x4D6C00. 1 call.
 *
 * Struct offsets:
 *   param_1[2] (object→+0x14=index), param_1[3] (object→+0xB38+index*4=counter)
 *
 * Cross-references:
 *   1 call from HighScoreEntry_ScalarDtor (0x44F073)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
