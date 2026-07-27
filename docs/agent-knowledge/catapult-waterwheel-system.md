# Catapult + Waterwheel System (Ghidra-verified, sess4080)

## Catapult System

### Catapult_ctor (0x437E10, __thiscall, 3 params: this, board, mesh)
- Calls Stands_ctor(this, mesh) — clones spatial trees into obj+0x18
- Sets vtable = 0x4D4F98
- obj+0x10D0 = board pointer (param_1)
- obj+0x10D4 = Level_RenderCtor(alloc 0x10D0) — collision/render Level (INTERNAL)
- obj+0x10D8/10DC/10E0 = position XYZ (init 0)
- obj+0x10E4 = 0 (tension)
- obj+0x10E8 = -1.0f (0xBF800000)
- obj+0x10F0 = 0 (launching flag)
- obj+0x10F4 = countdown (set to 50=0x32 by Catapult_Launch)
- obj+0x1104 = 17.0f (0x41880000)

### Catapult_Launch (0x434290, __fastcall, 1 param: this)
- obj+0x10F0 = 1 (launching)
- obj+0x10F4 = 0x32 (50 ticks countdown)

### vtable[11] = 0x437F10 — per-frame state machine
- If obj+0x10F0 (launching) == 0: skip (idle)
- If launching: decrement obj+0x10F4 (countdown)
- During countdown: wind-up animation (Gfx_ScaleY, Gfx_SetPosition, vtable[22]+[21])
- Plays Sound_Play3D at board+0x878+0x47C

### Catapult_Update (0x43F080, __fastcall, vtable[61])
- tension (obj+0x10E4) *= 1.25 per frame (0x4CF434)
- When tension >= 90.0 (0x4D03A0): reset to 90.0
- Calls Scene_ForEachBall_SetVelocity(0x419B70, board, x, y, z)
  - Iterates board+0x29D4 (ball AthenaList)
  - Calls Ball_CheckProximity(ball, x, y, z) for each ball
  - If ball within 500.0 (0x4CF3D8) distance: ball+0x744 = 1
- Sound_Play3D at board+0x878+0x47C

### TowerCollisionEvents (0x40DD68, vtable[29] of Tower board)
- __stricmp(mesh_name, "E:CATAPULTBOTTOM")
- If match AND ball+0x808 (param_1[0x202]) < 1:
  1. ball+0x808 = 1000 (cooldown)
  2. Iterate board+0x43B8 (Catapult AthenaList, count at +0x43BC)
  3. For each catapult: check catapult+0x10D4 == collision_mesh_ptr
  4. If match: catapult+0x10EC = ball_ptr (store ball)
  5. Catapult_Launch(catapult) — sets launching + 50 tick countdown
  6. Sound_PlayChannel(board+0x878+0x464)

### MOD BUGS FOUND:
1. case 35 creates DUPLICATE Level_RenderCtor at obj+0x10D4, overwriting ctor's internal one
   FIX: Remove the duplicate creation (lines 2029-2040)
2. Catapult collision Level at obj+0x10D4 is NOT added to collision list
   FIX: Add obj+0x10D4 to board+0x10EC (collision list) + sceneobj+0x18 (spatial tree)

## Waterwheel System

### Board offsets:
- board+0x4BA8 = WaterWheel MeshWorld* (primary swirl mesh)
- board+0x4BAC = WaterWheel RenderObject*
- board+0x4BB0/4/8 = WaterWheel position XYZ
- board+0x4BBC = scale value (init 0)
- board+0x4BC0 = Gfx_ScaleY parameter
- board+0x4BCC/0/4 = secondary (actual waterwheel) position XYZ

### Collision events:
- N:WATERWHEEL → ball+0xC3C=1 (on waterwheel flag)
- N:WHEELEMBED → ball+0x2E5=1 (embedded), ball+0x808=50 (0x32 = waterwheel embed mode)
- N:SWIRL → ball+0x778=1 (swirl active)

### Per-frame (DizzyBoard_Update 0x41D512):
1. Rotate mesh: angle -= 0.5/frame (0x4CF3F0)
2. Gfx_ScaleY, Gfx_SetPosition, vtable[22]+[21]
3. If ball on waterwheel: Ball_ApplyForceV2(ball, dirX, dirY, dirZ, 0.1)
   Force direction = ball position - waterwheel position (outward push)

### Ball_ApplyForceV2:
- ball+0xCA4/CA8/CAC += dir * magnitude
- Guards: ball+0x2F9==0, ball+0x2CC==0, ball+0x808==0, ball+0x2F0<0x51

### Constants:
- 0x4D0920 = 0.0001 (timer/magnitude)
- 0x4D0924 = 0.1 (force scale)
- 0x4D0928 = 10000.0 (max distance)
- 0x4CF3F0 = -0.5 (rotation speed per frame)

### MOD BUGS FOUND:
1. Waterwheel mesh rotates but NO force applied to ball
   FIX: Add force application in cEnt_waterwheel_update()
2. No collision events (N:WATERWHEEL, N:WHEELEMBED) checked
   FIX: Add proximity-based collision event checks (same pattern as Tarpit/Gluebie)
