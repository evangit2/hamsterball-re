/*
 * Function: TourneyMenu_HandleButton
 * Address: 0x00450360
 * Signature: void __thiscall ...(void *this, char *param_1)
 * Parameters:
 *   this: TourneyMenu* | param_1: char* button name
 *
 * Description:
 * Handles tourney menu button presses. 'PLAY': sets level at App+0x220+0x90, sets flag +0xCF4=1, MusicPlayer_SetTempoScale(0.5). If tournament lost: creates OkayDialog 'LOST TOURNAMENT' with rollback instructions. 'ROLL': rollback support. 1 DATA ref.
 *
 * Struct offsets:
 *   +0x878 (App→+0x220), +0xCF4 (flag=1), +0xCFC (level index), App+0x534 (MusicPlayer), App+0x220+0x90/+0x95/+0x96
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D8438
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
