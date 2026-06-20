/*
 * Function: RegisterDialog_Tick
 * Address: 0x0044fc90
 * Signature: void * __thiscall ...(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: RegisterDialog* | param_1: stored at +4 | param_2-4: position data
 *
 * Description:
 * Tick for RegisterDialog. Sets vtable 0x4D6E54. Copies position. Sets size=0 (+0x14). Random lifetime 200-300 (+0x18). Random drift speed 50-100×_DAT_004CF524 (+0x1C). 2 calls.
 *
 * Struct offsets:
 *   +0x04 (param_1), +0x08/+0x0C/+0x10 (position), +0x14 (size=0), +0x18 (lifetime 200-300), +0x1C (drift=(50-100)×_DAT_004CF524)
 *
 * Cross-references:
 *   2 calls from FollowBall_Update, Ball_Update
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
