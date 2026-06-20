/*
 * Function: Vec3_CopyUnlessEqual
 * Address: 0x00402BF0
 * Signature: void __thiscall Vec3_CopyUnlessEqual(void *this, undefined4 *param_1)
 *
 * Description:
 * Copies a 3-component vector from param_1 to this, but only if the pointers
 * are different (prevents self-copy). If this == param_1, the function is a no-op.
 *
 * This is a standard safe-copy utility used throughout the codebase to avoid
 * self-assignment issues.
 *
 * Layout:
 *   +0x00: X (float)
 *   +0x04: Y (float)
 *   +0x08: Z (float)
 *
 * Cross-references (14+ call sites):
 *   - Impossible_Level_Update — updating positions in the Impossible level
 *   - Scene_UpdateBallsAndState (2 calls) — ball position updates
 *   - MeshNode operations (3 calls) — mesh vertex/transform copies
 *   - SpatialTree_ForEach — spatial data copying
 *   - Transform_ScalarDtor — transform copying
 *   - Various collision and rendering functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_CopyUnlessEqual(void *this,undefined4 *param_1)

{
  if (this != param_1) {
    *(undefined4 *)this = *param_1;
    *(undefined4 *)((int)this + 4) = param_1[1];
    *(undefined4 *)((int)this + 8) = param_1[2];
  }
  return;
}
