/*
 * Function: Ball_RecordBest
 * Address: 0x00402400
 * Signature: void __thiscall Ball_RecordBest(void *this, int param_1)
 *
 * Description:
 * Records the ball's best streak/score if the new value exceeds the current best.
 *
 * Logic:
 *   1. Resets ball+0x2EC to 0 (clears a temporary counter)
 *   2. If param_1 > ball+0x2F4 (current best): updates ball+0x2F4 = param_1
 *   3. Otherwise: ball+0x2F4 = ball+0x2F4 (no-op, Ghidra decompilation artifact)
 *
 * This function is called whenever a scoring event occurs (hits, knockoffs,
 * collisions with game objects) to track the ball's best streak.
 *
 * Cross-references (7 call sites):
 *   - DispatchCollisionEvents (5 calls) — scoring events in the NoDizzy game mode
 *   - Arena_HandleCollision — arena collision scoring
 *   - HandleArenaCollisionEvents — spinner interaction scoring
 *
 * Struct offsets:
 *   ball+0x2EC: Temporary counter (reset to 0 each call)
 *   ball+0x2F4: Best streak value (int32, updated if param_1 > current)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_RecordBest(void *this,int param_1)

{
  *(undefined4 *)((int)this + 0x2ec) = 0;
  if (*(int *)((int)this + 0x2f4) < param_1) {
    *(int *)((int)this + 0x2f4) = param_1;
    return;
  }
  *(int *)((int)this + 0x2f4) = *(int *)((int)this + 0x2f4);
  return;
}
