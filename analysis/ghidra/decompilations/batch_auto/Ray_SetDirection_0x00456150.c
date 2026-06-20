/*
 * Function: Ray_SetDirection
 * Address: 0x00456150
 * Signature: void __thiscall ...(void *this, float *param_1)
 * Parameters:
 *   this: Ray* | param_1: float* (3-component direction vector)
 *
 * Description:
 * Sets ray direction. Copies 3 floats to +0xC (raw) and +0x18 (normalized). Vec3_NormalizeAndScale on +0x18 with 1.0. Computes length = sqrt(x²+y²+z²). If length < epsilon: sets +0x24=0. 3 calls.
 *
 * Struct offsets:
 *   +0x0C (raw direction), +0x18 (normalized direction), +0x24 (length)
 *
 * Cross-references:
 *   3 calls from collision detection
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
