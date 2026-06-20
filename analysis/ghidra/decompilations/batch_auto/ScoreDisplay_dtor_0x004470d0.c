/*
 * Function: ScoreDisplay_dtor
 * Address: 0x004470D0
 * Signature: void __fastcall ScoreDisplay_dtor(undefined4 *param_1)
 * Parameters:
 *   param_1: ScoreDisplay* this — the score display to destroy
 *
 * Description:
 * Destructor for ScoreDisplay objects. Steps:
 *   1. Sets vtable to PTR_ScoreDisplay_scalar_dtor (0x4D67E8)
 *   2. Frees two SSO strings:
 *      - +0x450 (affiliate key, capacity at +0x455): if capacity > 0xF, _free heap
 *      - +0x449 (serial key, capacity at +0x44E): if capacity > 0xF, _free heap
 *      Reset both to SSO mode (capacity=0xF, length=0)
 *   3. Calls RumbleBoard_CleanupTimer on +0x443
 *   4. Calls BaseObject_Init on 5 sub-objects: +0x43B, +0x434, +0x42D, +0x426, +0x41F
 *   5. Calls SceneObject_dtor (base class cleanup)
 *
 * Struct offsets:
 *   +0x41F/+0x426/+0x42D/+0x434/+0x43B: BaseObject sub-objects (7 bytes each)
 *   +0x443: Timer object
 *   +0x449: Serial key (SSO string, +0x44E = capacity)
 *   +0x450: Affiliate key (SSO string, +0x455 = capacity)
 *
 * Cross-references:
 *   Called from ScoreDisplay_scalar_dtor (0x447373) — UNCONDITIONAL_CALL
 *   Called from ScoreDisplay_dtor jump (0x4485E0) — UNCONDITIONAL_JUMP
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
