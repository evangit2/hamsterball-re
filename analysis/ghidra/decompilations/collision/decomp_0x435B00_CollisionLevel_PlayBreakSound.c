// Function: CollisionLevel_PlayBreakSound
// Address: 0x00435B00
// Calling Convention: __fastcall (param_1 = CollisionLevel object)
// Xrefs: Called from 0x0043C00A (likely inside a collision event handler)
//
// PURPOSE: Plays a 3D positional "break" sound at the collision level's current
// position and sets a cooldown flag. Used when breakable objects (bridges, glass,
// breakable platforms) are shattered by the ball.
//
// COLLISIONLEVEL OBJECT FIELDS:
//   param_1+0x10D0 → parent board/scene pointer
//     parent+0x878 → sound device manager
//       sound_mgr+0x4B4 → sound sample handle (BASS sample)
//   param_1+0x10D8/10DC/10E0 → X/Y/Z world position (floats)
//   param_1+0x10E4 → sound cooldown timer (set to 1.0f = 0x3F800000)
//
// The function is simple:
// 1. Dereference the chain: this → parent (+0x10D0) → sound manager (+0x878) → sample (+0x4B4)
// 2. Call Sound_Play3D(sample, x, y, z) to play the break sound at the object's position
// 3. Set cooldown flag at +0x10E4 to 1.0f (prevents re-trigger for some frames)

void __fastcall CollisionLevel_PlayBreakSound(int param_1)
{
  // ... decompiled body ...
}
