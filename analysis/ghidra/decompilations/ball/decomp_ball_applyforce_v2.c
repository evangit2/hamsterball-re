// =============================================================================
// Ball_ApplyForceV2 — Alt Force Application (0x4016F0)
// =============================================================================
//
// Secondary force application variant with gravity plane awareness.
// Called from Ball_Update when the ball receives external forces from
// the collision system, rather than direct player input.
//
// Address: 0x4016F0
// Calling convention: __thiscall (ECX = this)
// Parameters: this, float dir_x, float dir_y, float dir_z, float magnitude
//
// =============================================================================
// LOGIC
// =============================================================================
//
// GUARD CONDITIONS (all must be true for force to apply):
//   1. this[0x2F9] == 0 (not in tarpit state)
//   2. this[0x2CC] == 0 (force_disable flag is clear)
//   3. this[0x808] == 0 (freeze_counter == 0, not frozen)
//   4. this[0x2F0] < 81 (0x51) (frame counter within limit)
//
// FORCE MODIFIERS (applied in order):
//   1. First frame:  this[0x2F0] != 0 → magnitude *= 0.25 (0x4CF380)
//   2. In tube:      this[0x324] != 0 → magnitude *= 0.0 (0x4CF378) — NO FORCE IN TUBE
//   3. On ice:       this[0xC5C] != 0 → magnitude *= 0.01 (0x4CF374)
//                    AND: this[0xFC..0x104] = dir * 0.7 (0x4CF370) — weak side push
//   4. Is dizzy:     this[0xC4C] != 0 → magnitude *= 0.75 (0x4CF36C)
//
// VELOCITY ACCUMULATION:
//   this[0x170] += dir_x * magnitude   (X velocity += force_x)
//   this[0x174] += dir_y * magnitude   (Y velocity += force_y)
//   this[0x178] += dir_z * magnitude   (Z velocity += force_z)
//
// FACING DIRECTION (only if direction is non-zero):
//   gravity_plane (this[0x748]):
//     0 (flat/XZ):    angle = atan2(dir_x, dir_z)
//     1 (tilted):      angle = atan2(-dir_y, dir_z)
//     2 (vertical/XY): angle = atan2(dir_x, dir_y)
//   this[0x198] = angle
//   this[0x19C] = 1 (facing_updated flag)
//
// =============================================================================
// DIFFERENCES FROM Ball_ApplyForce (0x402650)
// =============================================================================
// - Ball_ApplyForce is the primary player-input force applier
// - Ball_ApplyForceV2 is used for collision-derived and system forces
// - V2 has the same multiplier chain but adds the "in tube" (0x324) check
//   which completely zeroes force when the ball is in a tube section
// - V2 also sets a weak lateral push (0xFC..0x104) when on ice
// - Both share the same gravity plane angle computation
//
// =============================================================================
// KEY OFFSETS
// =============================================================================
// this[0x2F9] = tar_state (tarpit flag)
// this[0x2CC] = force_disabled (1 = no force)
// this[0x808] = freeze_counter (>0 = frozen)
// this[0x2F0] = frame_counter (0 on spawn, >0 = not first frame)
// this[0x324] = in_tube_flag (1 = in tube section)
// this[0xC5C] = on_ice_flag (1 = on ice surface)
// this[0xC4C] = dizzy_flag (1 = dizzy from surface)
// this[0xFC..0x104] = lateral_push (XYZ, ice side force)
// this[0x170..0x178] = velocity (XYZ)
// this[0x748] = gravity_plane (0=flat, 1=tilted, 2=vertical)
// this[0x198] = facing_angle
// this[0x19C] = facing_updated_flag
//
// =============================================================================
// Force Multiplier Summary
// =============================================================================
// | Condition       | Offset | Multiplier | Notes                  |
// |-----------------|--------|------------|------------------------|
// | First frame     | 0x2F0 | 0.25       | Reduced initial push   |
// | In tube         | 0x324 | 0.0        | NO movement in tubes  |
// | On ice          | 0xC5C | 0.01       | Nearly zero force      |
// | Is dizzy        | 0xC4C | 0.75       | Reduced by 25%         |