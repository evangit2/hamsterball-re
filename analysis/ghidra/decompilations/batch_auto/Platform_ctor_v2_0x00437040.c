/*
 * Function: Platform_ctor (variant 2 — uses Level_Generic vtable)
 * Address: 0x00437040
 * Signature: void * __thiscall Platform_ctor(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, void *param_5)
 * Parameters:
 *   this: Platform* — the platform object being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D4 (position X)
 *   param_3: undefined4 — stored at this+0x10D8 (position Y)
 *   param_4: undefined4 — stored at this+0x10DC (position Z)
 *   param_5: void* — passed to Stands_ctor (parent scene for MeshWorld access)
 *
 * Description:
 * Second variant of Platform_ctor — uses a DIFFERENT vtable:
 * PTR_Level_GenericDeleteDtor (0x4D5950) instead of PTR_Rotator_DeletingDtor.
 * This suggests this Platform variant is a generic Level object, not a Rotator.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_5) — base class init
 *   2. Sets vtable to PTR_Level_GenericDeleteDtor (0x4D5950)
 *   3. Stores position at +0x10D4/+0x10D8/+0x10DC, parent Scene at +0x10D0
 *   4. Copies position to a secondary buffer at +0x10E0/+0x10E4/+0x10E8
 *      (original position stored for reset/respawn)
 *   5. Creates CollisionLevel (0x10D0 bytes) at +0x10EC
 *   6. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   7. Sets +0x10F1 = 0 (sink triggered = false)
 *   8. Sets +0x10F8 = 0 (timer/counter)
 *   9. Sets +0x10F0 = 1 (active = true)
 *   10. Sets +0x10F4 = 400 (timer duration or max value)
 *
 * Name verification: Vtable PTR_Level_GenericDeleteDtor (0x4D5950) is different
 * from the first Platform_ctor at 0x4363F0 (which uses 0x4D56A8/Rotator vtable).
 * This variant is a generic platform/level object, likely for static platforms.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0/+0x10E4/+0x10E8: original position (for reset)
 *   +0x10EC: CollisionLevel pointer (owned)
 *   +0x10F0: active flag (1)
 *   +0x10F1: sink triggered flag (0)
 *   +0x10F4: timer max (400)
 *   +0x10F8: timer counter (0)
 *
 * Cross-references:
 *   Called from CreatePlatformOrStands (0x413464) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
