/*
 * Function: RegisterDialog_Render
 * Address: 0x0044fa90
 * Signature: void * __thiscall ...(void *this, undefined4 param_1, int param_2)
 * Parameters:
 *   this: RegisterDialog* | param_1: undefined4 (stored at +4) | param_2: int (AthenaList source)
 *
 * Description:
 * Renders register dialog. Sets vtable 0x4D6E48. Gets random element from AthenaList via RNG_Rand. Copies position data (x,y,z from item+4/+8/+0xC). Sets size=25.0 (+0x14=0x41C80000). Random lifetime 25-50 (+0x18). 2 calls.
 *
 * Struct offsets:
 *   +0x04 (param_1), +0x08/+0x0C/+0x10 (position), +0x14 (size=25.0), +0x18 (lifetime 25-50)
 *
 * Cross-references:
 *   2 calls from Board_Master_Update, 0x41D59D
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
