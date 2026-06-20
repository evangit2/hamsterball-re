/*
 * Function: Material_Init
 * Address: 0x004561e0
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: Material* this — being zeroed
 *
 * Description:
 * Initializes Material struct to all zeros. Zeros 26 DWORDs (7 groups of 3 + 5 singles). Material is 0x64 bytes. 5+ calls from Ball_AdvancePositionOrCollision and collision detection.
 *
 * Struct offsets:
 *   0x00-0x64 (26 DWORDs, all zeroed)
 *
 * Cross-references:
 *   5+ calls from Ball_AdvancePositionOrCollision, collision functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
