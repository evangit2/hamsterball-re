/*
 * Function: Ball_FallUpdate
 * Address: 0x00408830
 * Signature: void __fastcall Ball_FallUpdate(int param_1)
 *
 * Description:
 * Updates the ball while it is in the "falling" state (after going off the edge
 * of a level). This is a lighter-weight physics update that handles gravity,
 * collision detection against respawn platforms, and velocity accumulation
 * during the fall.
 *
 * Logic:
 *   1. Decrements fall counter at +0x80C (counts down fall frames)
 *   2. Builds a temporary AthenaList with the fall collision mesh (+0x8B0)
 *   3. Sets CollisionMesh max_speed (+0xC70) = ball+0x188 (normal max speed)
 *   4. Saves current velocity to locals, zeroes ball velocity (temporary)
 *   5. Calls CollisionMesh vtable[6] (offset 0x18) — likely collision tree rebuild
 *   6. Allocates SpatialTree (0x20 bytes) with the fall collision list
 *   7. Sets SpatialTree max depth = 10, gravity scale = ball+0x278
 *   8. If fall counter expired and scale == 1.0: calls vtable[5] (offset 0x14)
 *      — likely collision query to check if ball has landed
 *   9. Allocates CollisionNode (0x14 bytes) with the ball list at Scene+0x29D4
 *   10. If not on ice (+0x324) and scale == 1.0: sets friction from +0x27C,
 *       calls vtable[5] for collision
 *   11. Overwrites CollisionMesh radius (+0xC78) with ball+0x284
 *   12. Copies trajectory from CollisionMesh+0xCA4, checks if Y component is
 *       near zero — if so, applies the trajectory as a launch
 *   13. Restores saved velocity
 *   14. Applies gravity: vel += dt * speed_scale * 1.5 * grav_dir
 *   15. Updates position: pos += vel * dt
 *   16. Lerps display position toward actual position (smoothing)
 *
 * Cross-references:
 *   - Referenced from Ball vtable (data ref) — called when ball+0xC4C flag is set
 *   - Not directly called — dispatched via vtable when Ball_Update detects
 *     the fall flag is active
 *
 * Struct offsets:
 *   ball+0x80C: Fall counter (decrements each frame)
 *   ball+0x188: Max speed (copied to CollisionMesh+0xC70)
 *   ball+0x278: Gravity scale
 *   ball+0x27C: Friction value
 *   ball+0x284: Ball radius (copied to CollisionMesh+0xC78)
 *   ball+0x324: On ice flag
 *   ball+0xC60: Scale (checked == 1.0)
 *   ball+0xC4C: Fall flag (checked by Ball_Update to dispatch here)
 *   CollisionMesh+0xCA4: Trajectory vector (checked for launch)
 *   Scene+0x8B0: Fall collision mesh
 *   Scene+0x29D4: Ball list for collision queries
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* ~6000 chars decompiled — see GhidraMCP for full body */
