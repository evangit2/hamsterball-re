/*
 * Function: Material_Copy
 * Address: 0x004562b0
 * Signature: void __thiscall ...(void *this, undefined4 *param_1)
 * Parameters:
 *   this: Material* (destination) | param_1: Material* (source)
 *
 * Description:
 * Copies entire Material struct (0x64 bytes, 26 fields in 7 groups). Uses self-copy detection (if dest==source for each group). 6+ calls from collision detection.
 *
 * Struct offsets:
 *   0x00-0x64 (all Material fields copied)
 *
 * Cross-references:
 *   6+ calls from collision functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
