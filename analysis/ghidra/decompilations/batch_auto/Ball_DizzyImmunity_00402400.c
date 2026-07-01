/*
 * Function: Ball_DizzyImmunity
 * Address: 0x00402400
 * Signature: void __thiscall Ball_DizzyImmunity(void *this, int param_1)
 *
 * Description:
 * Grants dizzy immunity: clears bounce_count, sets immunity timer to max(current, TIME).
 *
 * Logic:
 *   1. Resets ball+0x2EC to 0 (clears a temporary counter)
 *   2. If param_1 > ball+0x2F4 (current best): updates ball+0x2F4 = param_1
 *   3. Otherwise: ball+0x2F4 = ball+0x2F4 (no-op, Ghidra decompilation artifact)
 *
 * This function is called whenever a scoring event occurs (hits, knockoffs,
 * collisions with game objects) to grant temporary dizzy immunity.
 *
 * Cross-references (7 call sites):
 *   - DispatchCollisionEvents (5 calls) — scoring events in the NoDizzy game mode
 *   - ExpertCollisionEvents — arena collision scoring
 *   - HandleArenaCollisionEvents — spinner interaction scoring
 *
 * Struct offsets:
 *   ball+0x2EC: Temporary counter (reset to 0 each call)
 *   ball+0x2F4: Dizzy immunity timer (int32, frames — set to max(current, param_1))
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_DizzyImmunity(void *this,int param_1)

{
  *(undefined4 *)((int)this + 0x2ec) = 0;
  if (*(int *)((int)this + 0x2f4) < param_1) {
    *(int *)((int)this + 0x2f4) = param_1;
    return;
  }
  *(int *)((int)this + 0x2f4) = *(int *)((int)this + 0x2f4);
  return;
}
