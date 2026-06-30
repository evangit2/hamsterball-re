/*
 * Function: RaceResults_Ctor
 * Address: 0x0044aec0
 * Signature: void * __thiscall ...(void *this, undefined4 param_1)
 * Parameters:
 *   this: RaceResults* (vtable=0x4D6B10) | param_1: undefined4 (stored at +4)
 *
 * Description:
 * Constructor for RaceResults display. Calls ArenaScoreParticle_ctor, overrides vtable to 0x4D6B10. Initializes Vec3 at +0x28. Calls Matrix_Scale4x4 (0.5,0.5,0.5,1.0), stores result at +0x2C through +0x38. Calls Matrix_Identity. 1 call.
 *
 * Struct offsets:
 *   +0x28 (Vec3), +0x2C/+0x30/+0x34/+0x38 (scale matrix: 0.5,0.5,0.5,1.0)
 *
 * Cross-references:
 *   1 call from 0x43CDAF
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
