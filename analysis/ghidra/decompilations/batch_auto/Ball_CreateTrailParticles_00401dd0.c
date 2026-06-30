/*
 * Function: Ball_CreateTrailParticles
 * Address: 0x00401DD0
 * Signature: void __fastcall Ball_CreateTrailParticles(int param_1)
 *
 * Description:
 * Creates a circular ring of trail particles around the ball's current position.
 * The particles are distributed in a ring pattern using sine/cosine, and each
 * particle gets a randomized speed.
 *
 * Logic:
 *   1. Loops with local_50 starting at 0, incrementing by 0x28 (40 decimal, ~5.73°),
 *      until reaching 0x168 (360 decimal). This creates 9 particles (360/40=9)
 *      distributed evenly around the ball.
 *   2. For each particle:
 *      a. Gets the ball's transform axes (right/up/forward vectors) from
 *         ball+0x14→+0x878→+0x174→+0x744→offsets +0x5C through +0x70.
 *         These 6 floats represent 2 basis vectors (right and forward) for the ring plane.
 *      b. Computes ring position using Wave_Sin and Wave_Cos of the current angle.
 *         sin_component = right_vector * sin(angle) * ball_radius
 *         cos_component = forward_vector * cos(angle) * ball_radius
 *      c. Offsets the ring position by the ball's current position (+0x164/+0x168/+0x16C)
 *      d. Allocates a new RumbleScore particle (0x28=40 bytes) via operator_new
 *      e. Initializes it with ArenaScoreParticle_ctor, passing the parent object at ball+0x14→+0x878
 *      f. Sets position at particle+0x08/+0x0C/+0x10 (particle spawn position)
 *      g. Sets velocity direction at particle+0x14/+0x18/+0x1C
 *      h. Randomizes speed: speed = 1.0 / (RNG_Rand() + 20), then scales velocity
 *      i. Appends the particle to the trail list at ball+0x14→+0x3B00
 *
 * The ring radius is ball+0x284 (ball's max_speed/radius value).
 *
 * Cross-references:
 *   - Called from Ball_ApplyTrajectory (0x403750) — when ball is launched
 *   - Called from 0x409463 — likely in Ball_Update during high-speed movement
 *   - Called from Ball_Shatter (0x408D70) — when ball splits, each gets trail
 *   - Called from 0x440C77 — BounceBall area
 *
 * Struct offsets:
 *   ball+0x14: Board/parent object pointer
 *   ball+0x164/+0x168/+0x16C: Ball position (X, Y, Z)
 *   ball+0x284: Ball radius/speed value (used as ring radius)
 *   board→+0x878: Object containing transform basis
 *   board→+0x3B00: Trail particle list (AthenaList)
 *   particle+0x08/+0x0C/+0x10: Spawn position
 *   particle+0x14/+0x18/+0x1C: Velocity direction
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
