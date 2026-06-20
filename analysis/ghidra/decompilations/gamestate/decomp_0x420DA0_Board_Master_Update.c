// Function: Board_Master_Update
// Address: 0x00420DA0
// Calling Convention: __fastcall (param_1 = Master Race board object)
// Vtable: 0x4D12B0 (vtable[1] = Update slot)
// Xrefs: ONLY referenced from vtable entry at 0x4D12B4 (Master Race board vtable)
//        Confirmed unique: binary search for 0x00420DA0 finds exactly 1 hit in entire binary.
//
// PURPOSE: Per-frame update for the "Master Race" (Level 14) board. Master is the ONLY
// level with a custom Update method — all other levels use the default Scene_Update (0x419C00)
// as their vtable[1]. This custom update adds:
// 1. Scene_Update + two board vtable calls (+0x90, +0x94)
// 2. Random sparkle/particle effect (1-in-11 chance per frame): creates a 0x1C-byte effect
//    object (Ghidra mislabeled as "RegisterDialog_Render" — it's actually a SimpleList-based
//    particle that picks a random target from the board's effect list and stores its position).
//    Despite the name, this has NOTHING to do with the shareware nag screen — the real
//    registration dialog is at 0x447920. This function (0x44FA90) creates sparkle particles.
// 3. Ball-vs-mechanical-object collision: iterates all balls and checks 3D distance to
//    each mechanical object's center. If within activation radius, normalizes the collision
//    mesh velocity, plays 3D sound, spawns 3 trail particles.
// 4. Falling ball physics: gravity application (Y -= constant), random trail particle
//    spawning (1-in-15 chance), respawn trigger when ball drops below threshold.
//
// BOARD OBJECT FIELDS (offsets in int* units, i.e., multiply by 4):
//   param_1+0xA75/0xA76/0xB78 → ball list (AthenaList: header/count/data)
//   param_1+0xEC0 → UI effect/dialog list (AthenaList for particles)
//   param_1+0x1719 → effect target list (AthenaList of positions for sparkle effects)
//   param_1+0x1820/0x1821/0x1923 → mechanical object list (tippers/crushers)
//   param_1+0x21E0 → sound device pointer (for Sound_Play3D)
//
// BALL FIELDS (accessed via iVar14 pointer):
//   ball+0x164/0x168/0x16C → X/Y/Z position (float)
//   ball+0x1A4 → collision mesh pointer
//   ball+0x2CC → ball state flag (non-zero = falling/inactive)
//   ball+0x284 → ball radius
//   ball+0x2D0 → respawn height threshold
//   ball+0x2BC → "has been hit" flag (offset 700 = 0x2BC)
//   ball+0x810 → particle list (AthenaList, max 30 particles)
//   collision_mesh+0xCA4/CA8/CAC → position Vec3
//
// MECHANICAL OBJECT FIELDS:
//   obj+0x10E0/10E4/10E8 → center position (X/Y/Z)
//   obj+0x1100 → activation radius
//   obj+0x1104 → active flag (set to 1 when ball enters radius)
//
// SPARKLE EFFECT (mislabeled "RegisterDialog_Render" at 0x44FA90):
//   Creates 0x1C-byte SimpleList particle (vtable 0x4D6E48)
//   Picks random target from effect list (AthenaList_GetSize → RNG_Rand)
//   Copies target position (3 floats at +0x08/+0x0C/+0x10)
//   Sets lifetime = 25.0f (+0x14 = 0x41C80000) + random(25) offset
//   Appended to board's effect list (param_1+0xEC0)
//
// TRAIL PARTICLE (mislabeled "RegisterDialog_OnKey" at 0x44FB50):
//   Creates 0x1C-byte SimpleList particle at ball position
//   Sets lifetime = 25.0f + random(25) offset
//   Appended to board's effect list (param_1+0xEC0)
//
// PHYSICS:
// - When ball+0x2CC != 0 (falling): apply gravity (Y -= DAT_004cf380),
//   1-in-15 chance to spawn trail particle at ball position, check if Y < respawn threshold
// - When ball+0x2CC == 0 (active): check distance to each mechanical object,
//   if within radius: normalize collision mesh velocity, play 3D sound,
//   spawn 3 particles, set object active flag
//
// PARTICLES: 5-element float structs (3 pos + 2 velocity/extra), allocated with
// operator_new(0x14), velocity normalized via _DAT_004cf310 magnitude check.
// Max 30 particles per ball (AthenaList_GetSize < 0x1E check).

void __fastcall Board_Master_Update(int *param_1)
{
  // ... decompiled body ...
}
