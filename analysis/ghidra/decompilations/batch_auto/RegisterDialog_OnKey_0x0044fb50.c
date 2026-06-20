/*
 * Function: RegisterDialog_OnKey
 * Address: 0x0044fb50
 * Signature: void * __thiscall ...(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: RegisterDialog* | param_1: stored at +4 | param_2-4: position data (x,y,z)
 *
 * Description:
 * Key handler for RegisterDialog. Sets vtable 0x4D6E48. Stores param_1 at +4. Copies position (param_2-4) to +8/+0xC/+0x10 if different. Sets size=25.0. Random lifetime 25-50. 2 calls.
 *
 * Struct offsets:
 *   +0x04 (param_1), +0x08/+0x0C/+0x10 (position), +0x14 (size=25.0), +0x18 (lifetime 25-50)
 *
 * Cross-references:
 *   2 calls from Board_Master_Update, 0x41D779
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
