/*
 * Function: RumbleScore_ctor
 * Address: 0x0044ad50
 * Signature: void * __thiscall ...(void *this, undefined4 param_1)
 * Parameters:
 *   this: RumbleScore* (vtable=0x4D6AEC) | param_1: undefined4 (stored at +4)
 *
 * Description:
 * Constructor for RumbleScore (arena score display). Stores param_1 at +4, sets vtable 0x4D6AEC. Initializes SSO string at +0x14. Sets scale=1.0f (+0x20). Randomly selects one of 3 scale multipliers (0.02, 0.03, 0.04) via RNG_Rand(0-2). 4+ calls.
 *
 * Struct offsets:
 *   +0x04 (param_1), +0x14 (SSO string), +0x20 (scale=1.0), +0x24 (random scale mult: 0.02/0.03/0.04)
 *
 * Cross-references:
 *   4+ calls from Ball_CreateTrailParticles, CreateSpinner, RumbleObject_Update, RaceResults
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
