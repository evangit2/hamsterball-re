/*
 * Function: Ball_SetupCollisionRender
 * Address: 0x004015B0
 * Signature: void __fastcall Ball_SetupCollisionRender(int param_1)
 *
 * Description:
 * Sets up collision-related rendering and sound effects for a ball.
 * param_1 is the Ball object pointer.
 *
 * Logic:
 *   1. Checks ball+0x324 flag — if non-zero, skip entirely (ball already set up
 *      or collision rendering disabled)
 *   2. Gets the Scene pointer from ball+0x10
 *   3. Reads Scene+0x440 (CollisionLevel pointer). If non-null:
 *      a. Gets a sound channel via Sound_GetNextChannel(CollisionLevel)
 *      b. Stores channel at ball+0x258 (600 decimal)
 *      c. Calls Scene_RenderIfVisible(channel) — renders the collision object if visible
 *      d. Calls Sound_Play3DAtPosition(ball+0x258) — plays 3D positioned sound
 *   4. Resets ball+0x25C to 0 (clears some collision state)
 *   5. Checks Scene→Scene+0x220→+0x8 (a level/state type). If == 0xD (13):
 *      a. Reads Scene+0x4FC (another sound/collision object). If non-null:
 *      b. Gets sound channel, stores at ball+0xC54
 *      c. Renders if visible + plays 3D sound
 *
 * The 0xD check likely refers to a specific level type (possibly Impossible Race
 * which is level 13/15 in the track list, or an arena mode ID).
 *
 * Cross-references:
 *   - Called from Ball_ctor2 (0x4039E0) during ball construction
 *   - Referenced in vtables at 0x4CF318, 0x4D5D28, 0x4CF564 (Ball vtable entries)
 *
 * Struct offsets used:
 *   ball+0x10:    Scene pointer
 *   ball+0x258:   Sound channel (primary collision)
 *   ball+0x25C:   Collision state (reset to 0)
 *   ball+0x324:   Flag (skip if non-zero)
 *   ball+0xC54:   Sound channel (secondary collision, level 0xD only)
 *   scene+0x220:  Level state object
 *   scene+0x440:  CollisionLevel pointer
 *   scene+0x4FC:  Secondary collision/sound object
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_SetupCollisionRender(int param_1)

{
  int iVar1;
  
  if (*(char *)(param_1 + 0x324) == '\0') {
    iVar1 = *(int *)(*(int *)(param_1 + 0x10) + 0x440);
    if (iVar1 != 0) {
      iVar1 = Sound_GetNextChannel(iVar1);
      *(int *)(param_1 + 600) = iVar1;
      if (iVar1 != 0) {
        Scene_RenderIfVisible(iVar1);
        Sound_Play3DAtPosition(*(int *)(param_1 + 600));
      }
    }
    *(undefined4 *)(param_1 + 0x25c) = 0;
    if ((*(int *)(*(int *)(*(int *)(param_1 + 0x10) + 0x220) + 8) == 0xd) &&
       (iVar1 = *(int *)(*(int *)(param_1 + 0x10) + 0x4fc), iVar1 != 0)) {
      iVar1 = Sound_GetNextChannel(iVar1);
      *(int *)(param_1 + 0xc54) = iVar1;
      if (iVar1 != 0) {
        Scene_RenderIfVisible(iVar1);
        Sound_Play3DAtPosition(*(int *)(param_1 + 0xc54));
      }
    }
  }
  return;
}
