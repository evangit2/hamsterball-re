/*
 * Function: Scene_StartCountdown
 * Address: 0x00437130
 * Signature: void __thiscall Scene_StartCountdown(void *this, int param_1)
 * Parameters:
 *   this: Scene*/Board* — the scene/board starting the countdown
 *   param_1: int — pointer to a Ball object (the ball that triggered the countdown).
 *     Accesses param_1+0x18 (ball type, -1 = no ball), param_1+0x14C (ball state flag),
 *     param_1+0x2FC (float, set to 1.0)
 *
 * Description:
 * Starts the pre-race countdown sequence for a level. Only activates if:
 *   1. +0x10F1 == 0 (countdown not already started)
 *   2. +0x10F2 == 0 (countdown not already finished)
 *   3. param_1 != 0 (valid ball pointer)
 *
 * When triggered:
 *   1. Sets +0x10F1 = 1 (countdown started = true)
 *   2. Sets +0x10F2 = 0 (countdown finished = false)
 *   3. Sets +0x10F4 = 400 (countdown timer, ~6.67 seconds at 60fps)
 *   4. If ball type at param_1+0x18 == -1 (no specific ball), sets timer to 0x32 (50, ~0.83s)
 *   5. Stores ball pointer at +0x10F8
 *   6. Sets ball state flag at param_1+0x14C = 1 (ball is now in countdown)
 *   7. Sets ball timer at param_1+0x2FC = 1.0 (float, countdown progress)
 *
 * Called from collision handlers when a ball touches a trigger zone or reaches
 * a specific level section that starts the race countdown.
 *
 * Struct offsets:
 *   +0x10F1: countdown started flag (0→1)
 *   +0x10F2: countdown finished flag (0)
 *   +0x10F4: countdown timer (400 or 50)
 *   +0x10F8: triggering ball pointer
 *
 * Cross-references:
 *   10+ calls from various collision handlers and level setup functions:
 *   SinkPlatformArenaCollisionEvents (0x413C00), and others at 0x413FA0, 0x4143AE,
 *   0x414660, 0x414ED9, 0x4151C0, 0x4155B6, 0x41624F, etc.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
