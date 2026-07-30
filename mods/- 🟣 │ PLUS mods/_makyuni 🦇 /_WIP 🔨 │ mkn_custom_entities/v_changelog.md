## v55m_42 — Play dropin sound via BASS directly

- Load real BASS function pointers: BASS_SampleLoad, BASS_SampleGetChannel, BASS_ChannelPlay, BASS_SampleFree.
- Load 'sounds\dropin' sample once per level at catapult spawn.
- Play it on launch via BASS_SampleGetChannel + BASS_ChannelPlay.
- Free sample on level unload to avoid leak.

## v55m_41 — Diagnostic build: catapult sound disabled

- Used to confirm if the ntdll crash is caused by sound load/play.

## v55m_40 — Fix sound crash: use correct board+0x464 slot for dropin

- Native catapult sound loading uses board+0x464 for 'sounds\dropin'.
- v55m_39 wrote to board+0x460, corrupting another sound slot.

## v55m_39 — Use native sound manager virtual call for dropin sound

- Replaced manual Sound_LoadOggOrWav with board+0x22C sound manager vtable[24].
- Native pattern loads 'sounds\dropin' into board+0x460 slot, then Sound_Play3D.

## v55m_38 — Fix catapult sound crash: use App+0x460 as SoundList

- v55m_37 passed App* (board+0x878) to Sound_LoadOggOrWav; now uses App+0x460.
- Added diagnostic logs for sound_list and loaded channel.

## v55m_37 — Stronger catapult launch + safe dropin sound

- Increased launch force from 35/25 to 75/45 (closer to native 90 velocity).
- Load dropin sound via Sound_LoadOggOrWav(board+0x878) instead of cached App+0x460 pointer.
- Play via Sound_Play3D to avoid Sound_PlayChannel crash.

## v55m_36 — Disable catapult sound + star trail to isolate crash

- Catapult trigger now fires correctly.
- Removed sound play and star trail writes (caused 0x00000000 crash after launch).
- Keep launch force only for stability test.

## v55m_35 — Restore v55m_28m radius catapult trigger, remove SEH hook

- Removed `board+0x43B8` append (caused heap corruption on non-Tower levels).
- Removed `install_bonk_collision_hook()` (SEH trampoline at `DispatchCollisionEvents` caused Draw crashes at 0x452376).
- Restored Present-hook radius trigger from v55m_28m.

## v55m_34 — Remove unsafe catapult MeshWorld scan

- v55m_33 scanning the catapult collision Level's MeshWorld caused crash at 0x45237E.
- Replaced with simple `cat_col_obj` pointer log.

## v55m_33 — Add catapult diagnostic logs

- Log whether `E:CATAPULTBOTTOM` mesh exists in the spawned catapult collision level.
- Log when the player ball collides with `E:CATAPULTBOTTOM`.
- Log when `E:CATAPULTBOTTOM` triggers `Catapult_Launch`.

## v55m_32 — Fix catapult E:CATAPULTBOTTOM trigger

- Install DispatchCollisionEvents hook when a Catapult is spawned (not just Bonk).
- Match `collision_data[0]` (entity pointer) against `catapult+0x10D4`, not `collision_data[1]`.

## v55m_31 — Revert collision-site hooks to v55m_29 baseline

- Reverted v55m_30/v55m_30a collision-site hook rewrite; it caused heap corruption (crash 0x45FB03).
- Restored native E:CATAPULTBOTTOM DispatchCollisionEvents approach from v55m_29.

# Version Changelog

## v55m_29 — Catapult uses native E:CATAPULTBOTTOM collision event

- **Problem:** v55m_28m catapult fired via radius trigger, but the zone was too wide and did not match the native Tower behavior.
- **Fix:**
  - Removed the manual radius trigger + force push in `cEnt_catapult_present_check`.
  - Re-enabled adding the spawned `Catapult_ctor` object to the native catapult update list at `board+0x43B8`, so the original wind-up/launch state machine (`Catapult_vtable11` / `Catapult_Update`) runs.
  - In the `DispatchCollisionEvents` detour, `E:CATAPULTBOTTOM` now directly calls `Catapult_Launch(obj)` on the matching tracked catapult instead of just setting a `collided` flag.
  - Falls back to launching the closest tracked catapult if the collision pointer does not match exactly.
- **Behavior:** Entering the `E:CATAPULTBOTTOM` collision mesh plays the native 50-frame wind-up animation, then launches the ball with the original Tower force/direction.

## v55m_28m — Catapult trigger diagnostics + wider zone

- **Problem:** v55m_28l catapult tracked and heartbeat logged, but no TRIGGER/LAUNCH lines. User entered the catapult and nothing happened.
- **Diagnosis:** `custom_entities_catapult.log` showed heartbeat every 60 frames for 780+ frames with no trigger. Either the ball was outside the radius 80 / dy [-100,+40] window, or `get_ball_ptr` returned a non-player ball.
- **Fix:**
  - Widened trigger zone to radius 120, dy [-120,+80].
  - Widened reset zone to radius 250, dy [-180,+120] (must be larger than trigger).
  - Added per-frame proximity logging: ball position, catapult position, horizontal distance, dy, trigger/reset flags, cooldown, `was_in_zone`.
  - Logging now happens every 30 frames and also whenever the ball is inside the reset zone, so the user can see exactly why the trigger does or does not fire.
- **Crash test:** pending.

## v55m_14 — Chomper crash fix (0000:00000010 during Draw)

- **Root cause:** Chomper added `coll` (Level_RenderCtor result) to
  board+0xCD4 (render list) and sceneobj+0x1C (spatial tree).
  The game's Draw iterates the render list and calls vtable[2]
  (SceneObject_BuildStrips) on each item. BuildStrips calls
  Font_RenderToTextureComplex with the MeshWorld's vertex count.
  The Chomper's `coll` has an EMPTY MeshWorld (Level_LoadMeshes
  creates a new MeshWorld via MeshWorld_ctor_simple but the parent
  mesh_world+0x08 is NULL because vtable[14] loads .MESHWORLD binary
  data without setting +0x08). With 0 vertices, Font_RenderToTextureComplex
  creates a 0x0 D3D texture -> NULL -> reads NULL+0x10 -> crash at
  0x00000010.
- **Fix:** Removed coll from board+0xCD4 (render list), board+0x8B8
  (update list), and sceneobj+0x1C (spatial tree). The Present hook
  (cEnt_chomper_update) already handles all rendering via vtable[0x16]
  (SceneObject_CallUpdate) + vtable[0x15] (SceneObject_CallRender) with
  Timer_Init/Gfx_ScaleZ. No game render pipeline involvement needed.
- **Ghidra analysis:** LoadMeshWorld (0x45DE30) has two paths:
  (1) File exists: calls vtable[14] (0x4629E0, binary file loader) +
  vtable[15] (0x460DA0, Scene_RenderFrame). vtable[14] sets this+0x47C
  = self, this+0x431=1, this+0x434=Timer, but does NOT set this+0x08
  (MeshWorld*). (2) File not found: creates MeshWorld at +0x08 via
  operator_new(0x488)+MeshWorld_ctor+MeshWorld_Parse. Level_LoadMeshes
  reads parent+0x08 to copy mesh data -> NULL -> empty child MeshWorld.

## v55j_8 — Gluebie: match native Dizzy behavior (Ghidra-verified)

- **Gluebie proximity behavior fixed to match native DizzyBoard_Update exactly.**
  Deep Ghidra decompilation of DizzyBoard_Update (0x41D512), Gluebie_ctor
  (0x437CB0), Gluebie vtable[11] (0x43ECC0), and Dizzy_CreateDynamicObjects
  (0x40A5F0) revealed 5 behavioral differences:
  - **BUG 1 (wrong flag):** Mod set ball+0x260 (tar render flag) — native
    Gluebie does NOT touch ball+0x260. That flag is set by Ball_Update when
    the ball physically touches the tar SURFACE (3.0 units). Mod was showing
    tar splotch at 45-60 units distance (way too early) and hiding it when
    leaving range. Native sets ball+0x2BC (sound/particle cooldown) instead.
  - **BUG 2 (wrong clear):** Mod cleared ball+0x260=0 when ball left range.
    Native NEVER clears ball+0x2BC — it stays 1 until ball dies/respawns.
  - **BUG 3 (wrong radius):** Mod used hardcoded 60.0. Native uses
    obj+0x1100 * 60.0, where Gluebie_ctor inits +0x1100 to
    (RNG(25)+75)*0.01 = 0.75-1.0, giving radius 45-60.
    Mod comment said "ctor doesn't init +0x1100" — WRONG, it does.
  - **BUG 4 (double processing):** Present hook ran Gluebie check on Dizzy
    Race, where native DizzyBoard_Update ALREADY handles it → double
    velocity scaling. Added gluebie_is_dizzy() check to skip on Dizzy.
  - **BUG 5 (missing cooldown flag):** Mod used internal static cooldown
    counter instead of ball+0x2BC. Native checks ball+0x2BC==0 before
    playing sound, then sets ball+0x2BC=1 (once per entry, no counter).
- **Velocity scaling confirmed correct:** Native does normalize velocity,
  multiply by (speed * 0.95) / speed = 0.95, which IS just *= 0.95 with
  zero-velocity guard. Mod's simple *= 0.95 is functionally equivalent.
- **Constants verified (Ghidra memory reads):**
  - _DAT_004d0930 = 60.0 (outer radius multiplier)
  - _DAT_004d092c = 0.95 (velocity scale factor)
  - _DAT_004cf368 = 0.0 (epsilon)
  - _DAT_004cf380 = 0.25 (tar sink rate)
  - _DAT_004cf480 = 75.0 (radius RNG offset)
  - _DAT_004cf524 = 0.01 (radius RNG scale)
- **Not yet implemented (native has, mod doesn't):**
  - Tar splotch particles (3x operator_new(0x14), random direction,
    appended to ball+0x810 AthenaList, max 30). Visual only, no gameplay
    impact. Will add if user reports missing particles.
- **Crash test:** PASS (12.1s, no crash, DLL restored).

## v55g — Catapult: full system port (solid + launch + state machine)

- **Catapult (ai_type 35) now has full native behavior ported.**
  - **Problem:** Catapult was spawning but non-solid and static — no collision,
    no wind-up, no launch, no per-frame state machine.
  - **Root cause:** Catapult_ctor calls Stands_ctor which clones spatial trees
    into obj+0x18 (making it solid), but the collision/render Level was never
    created. Without it, the collision object at obj+0x10D4 was NULL, so the
    collision list (board+0x10EC) was empty — ball passed through.
  - **Fix — 5 components ported from native (Ghidra-verified):**
    1. **Collision/render Level:** Create via `Level_RenderCtor(mesh)` and store
       at obj+0x10D4 (same pattern as TipperVisual). Added to collision list
       (board+0x10EC) and scene collision (sceneobj+0x18).
    2. **Per-frame state machine:** `Catapult_vtable11` (0x437F10, vtable[11])
       handles wind-up + release. Native game calls this via Board_UpdateRaceState,
       but Catapult is NOT in the Scene_Update list (board+0x8B8). Added manual
       per-frame call in `entity_thread` for each tracked Catapult.
    3. **E:CATAPULTBOTTOM trigger:** Native game only checks this on Tower Race
       (race 4). Instead of hooking DispatchCollisionEvents (SEH trampoline crash
       risk — confirmed in v53g-2), use per-frame proximity check: when ball is
       within 40 units of catapult base (Y offset -10 for bottom plate), call
       `Catapult_Launch` (0x434290) which sets launching flag + 50-tick countdown.
    4. **Tracking:** Catapult objects tracked in `g_catapults[]` array (max 16)
       for per-frame updates. Reset on level unload.
    5. **Collision list fix:** Updated col_off logic to include type 35
       (Catapult) alongside PopCylinder(0) and Rotator(1-6) for collision
       object at +0x10D4.
  - **Functions ported (all Ghidra-verified):**
    - `Catapult_ctor` (0x437E10, 3 params: this, board, mesh) — already called
    - `Catapult_Launch` (0x434290, __fastcall, 1 param: this) — trigger
    - `Catapult_vtable11` (0x437F10, __fastcall, 1 param: this) — state machine
    - `Catapult_Update` (0x43F080, __fastcall, 1 param: this) — vtable[61], animation
      (called by render loop via board+0xCD4 render list, no manual call needed)
  - **Note:** `logf` not used in per-frame Catapult code (dangling pointer after
    initial fclose in entity_thread — pre-existing issue).
  - **AI list entry updated:** type changed from 0 to 35 (Catapult).

## v55f — WaterWheel: mesh loaded + rotated per-frame (no entity)

- **WaterWheel (ai_type 26) now loads its mesh and rotates it each frame.**
  - Native game (DizzyBoard_ctor at 0x41D067) creates a mesh via `MeshWorld_ctor("Levels\\Level3-WaterWheel")`
    stored at `board+0x4BA8`, position at `board+0x4BB0`, angle at `board+0x4BBC`.
  - DizzyBoard_Update (0x41D512) rotates: `angle -= 0.5/frame` (constant at 0x4CF3F0),
    then applies via `Gfx_RotateY(stack_matrix, angle)` (0x457C90) + mesh vtable[22] (SetTransform)
    + mesh vtable[21] (SetPosition).
  - **Old behavior:** spawned PopCylinder with the mesh path (wrong — non-solid, static).
  - **New behavior:** creates mesh via `MeshWorld_ctor`, stores in `g_waterwheels[]` array,
    rotates per-frame via `cEnt_waterwheel_update()` which calls `Gfx_RotateY` + mesh vtable[22]+[21].
  - Mesh is added to board render list (`board+0xCD4`) and scene spatial tree (`sceneobj+0x1C`)
    for visibility and collision.
- **Added `cEnt_waterwheel_update()` — per-frame rotation.**
  - Calls `Gfx_RotateY(rot_matrix, angle)` to build Y-rotation matrix.
  - Calls `mesh->vtable[22]()` (SetTransform, no params) then `mesh->vtable[21](&rot_matrix)` (SetPosition, 1 param).
- **AI list entry updated:** mesh path stays `levels\\Level3-WaterWheel` (used by `MeshWorld_ctor`).

## v55e — TarBubble: no entity, position-only marker

- **TarBubble no longer spawns a PopCylinder entity.**
  - Native game has NO TarBubble constructor — S1 ref points named "TarBubble"
    are stored in `board+0x4790` AthenaList as position markers.
  - DizzyBoard_Update (0x41D512) creates a collision traversal object (0x44FA90)
    that sinks the ball 0.25/frame when inside the tar radius.
  - Old behavior: spawned PopCylinder with `meshes\tarbubble` mesh (wrong).
  - New behavior: stores position in `g_tarbubble_pos[]` array, no entity spawned.
- **Added `cEnt_tarbubble_proximity_check()` — replicates native tar sinking.**
  - Per-frame check: iterates balls, computes 3D distance to each TarBubble position.
  - When ball is within 3.0 units: sinks ball Y by 0.25/frame (native constant 0x4CF380),
    sets `ball+0x2CC` (in_tar flag — disables control, decays spin 0.85x/frame),
    sets `ball+0x2BC` (tar render flag).
  - Radius 3.0 matches native Ball_Update tar surface contact distance.
- **AI list entry changed:** `Tarbubble` mesh path → `NULL` (no mesh file needed).

## v55d — Gluebie tar sound + proximity fix

- **Gluebie tar sound: Fixed crash + implemented sound playback.**
  - Root cause: `Sound_Play3D` typedef was missing the 4th parameter (float scale=1.0).
    Native function has `RET 0x10` (4 params = 16 bytes). Old typedef only passed 3
    params → stack corruption → crash at 0x4065F2 inside Ball_ctor.
  - Sound is now queued via `g_gluebie_sound_pending` flag when ball enters Gluebie
    range, then played from the background thread loop with proper 4-param call:
    `Sound_Play3D([App+0x484], x, y, z, 1.0f)`.
  - App accessed via `board+0x878` (same as native DizzyBoard_Update at 0x41D9B3).
  - Added `IsBadReadPtr` safety checks on sound channel pointer.
- **Gluebie proximity radius: Changed from 45-60 to 3.0 (user request).**
  - Native has TWO proximity checks:
    1. `DizzyBoard_Update`: `obj+0x1100 * 60.0` = 45-60 units (center-to-center, velocity slowdown)
    2. `Ball_Update`: `3.0` units (distance to tar surface, sets tar flag)
  - The 45-60 value was too large for non-Dizzy levels (smaller Gluebies).
  - Using 3.0 as requested — matches the native inner zone.
- **Bridgeslam sound: Also fixed to pass 4th param (1.0f) + IsBadReadPtr check.**
  - Same `Sound_Play3D` signature bug affected bridgeslam sound (latent crash).

## v55c — Gluebie collision + behavior fix

- **Gluebie (ai_type 43): Fixed missing collision + behavior + cross-level proximity.**
  - **Root cause:** Gluebie was ai_type=0 (PopCylinder fallback). No Gluebie_ctor
    was called, so it had no Gluebie vtable (0x4D4F38), no vtable[11] update
    function (0x43ECC0), and no proximity behavior.
  - **Fix 1 (ctor):** Added ai_type 43 with Gluebie_ctor (0x437CB0, 0x110C bytes).
    - Gluebie_ctor calls Stands_ctor (clones spatial trees from Level3-Gluebie mesh)
    - vtable[11] (0x43ECC0) handles rendering + animation (scale, position, particles)
    - Position stored at obj+0x10D4/10D8/10DC (matching native Dizzy_CreateDynamicObjects)
  - **Fix 2 (cross-level proximity):** Native Gluebie behavior is in DizzyBoard_Update
    which iterates board+0x4378 (Dizzy-only AthenaList — Tower/Expert/Toob use 0x4378
    as a Level pointer, so it can't be safely initialized on other levels).
    Added mod-side proximity check (cEnt_gluebie_proximity_check) that runs every
    frame from the mod's background thread:
    - Iterates board+0x29D4 (ball AthenaList) and mod's g_gluebie_objs array
    - Checks 3D distance: sqrt((gluebie+0x10E0 - ball+0x164)^2 + ...) < gluebie+0x1100 * 60.0
    - If in range: scales ball velocity (collisionMesh+0xCA4/CA8/CAC) by 0.95
    - Sets gluebie+0x1104 = 1 (active flag)
    - ball+0x1A4 = collision mesh pointer (set by Ball_ctor via CollisionMesh_ctor)
  - **Native Gluebie behavior** (DizzyBoard_Update, decompiled):
    - Iterates board+0x4378 (Gluebie list)
    - Checks ball proximity: distance(gluebie_pos, ball_pos) < gluebie+0x1100 * 60.0
    - If ball in range: scales ball velocity by 0.95, plays sound, creates 3 particles
    - Sets gluebie+0x1104=1 (active flag)

## v55b — Tipper collision fix

- **Tipper (ai_type 37): Fixed missing collision + crash on level start.**
  - **Crash cause:** TipperVisual_Attach (0x465200) was declared as `__cdecl` but is
    actually `__thiscall` (starts with `MOV ESI,ECX`, `RET 0x4`). With `__cdecl`,
    ECX was not set to the visual pointer → ESI = garbage → access violation reading
    ESI+0x431 at 0x465203. Crash report: 0001:00064203.
  - **Collision root cause:** Tipper_ctor creates the behavior object but sets
    obj+0x10D4 = 0. The Tipper's vtable[11] (0x437A40) is a 4-state machine that
    dynamically adds/removes obj+0x10D4 (the TipperVisual) from the scene collision
    list (board+0x8B0+0x18) as the tipper raises/lowers. Without TipperVisual,
    obj+0x10D4=0 (NULL) → AthenaList_Append(board+0x8B0+0x18, NULL) → no collision.
  - **Fix 1 (crash):** Changed TipperVisual_Attach typedef from `__cdecl` to `__thiscall`.
  - **Fix 2 (collision):** After Tipper_ctor, create TipperVisual matching native
    Dizzy_CreateDynamicObjects flow:
    1. Level_RenderCtor from Level3-Tipper mesh → render Level
    2. TipperVisual_ctor from render Level → visual object
    3. Store visual at obj+0x10D4 (DWORD index 0x435)
    4. TipperVisual_Attach(visual, obj) — links visual to behavior
  - **Native vtable[11] state machine:**
    - State 0: Countdown → AthenaList_Append(collision_list, obj+0x10D4) → state 1
    - State 1: Raise animation (scale × 1.05/frame) → state 2
    - State 2: Wait (100 frames) → state 3
    - State 3: Lower animation (scale × 0.85/frame) → AthenaList_Remove(collision_list, obj+0x10D4) → state 0

## v55 — Major fix: Static Swirl + crashes

- **Static Swirl fix (10 entities): 8ball, Bell, Chomper, Fan, Funball, Glassbreaker, Judge, Mag, Sawblade, Tarbubble**
  - Root cause: if/else-if chain bug (fixed in v54) was still sending .MESH entities
    through the .MESH swap code with Swirl mesh instead of their correct mesh.
  - v55 fix: Removed ai_types 10, 11, 15, 22 from path=NULL override. These entities
    have .MESH file paths (meshes\*) and should go through the .MESH swap code which:
    1. Loads the correct .MESH via MeshNode_ctor
    2. Loads Swirl as PopCylinder base (proper vtable, position, collision)
    3. Swaps obj+0x08 to the entity's MeshWorld (correct visual model)
    4. NO vtable override (prevents stack corruption from v54d crash)
  - Result: All 10 entities should now show their correct mesh model instead of Static Swirl.

- **Crash fixes (11 entities → 4 crash patterns resolved):**
  - MeshArchive_ctor crash (Bridge, Chomper, Wobbly): Changed ai_type to 0 (PopCylinder fallback).
    These entities' native constructors (GameLevel_ctor, BreakBridge_ctor) call MeshArchive_ctor
    which crashes without board pre-loaded mesh data. Now they use PopCylinder with their .MESHWORLD mesh.
  - Level_ctor crash (8Ball, Spinner): Fixed by removing ai_type 15 and 27 from path=NULL override.
    Now .MESH swap code handles them — no native constructor called.
  - MeshWorld_Parse crash (Flag, Flag2): Changed ai_type from 14 (Wavy_ctor) to 0 (PopCylinder).
    Flag.MESHWORLD doesn't exist, causing Wavy_ctor to crash during parsing.
  - Gear/Looper/Cloudscape/Drawbridge crashes: Changed ai_type to 0 (PopCylinder fallback).
    Native constructors crash without board pre-loaded data.

- **Chrome fix:** Changed mesh path from _default.MESHWORLD to Sphere.MESH.

- **Also includes all v54d fixes:**
  - if/else-if chain bug fix (path=NULL overrides now work)
  - Dizzy Arena double-Swirl fix (my_stristr for "Dizzy" in level name)
  - Bell/Fan/SawBlade case handlers with .MESH swap (no vtable override)

- Crash test: 38.8s OK

- **CRASH FIX: Removed vtable overrides for Bell/Fan/SawBlade**
  - v54 added vtable overrides to give PopCylinder objects native Bell/Fan/SawBlade behavior.
  - Crash at 0x0046186E (inside Level_ctor) — mid-instruction EIP = stack corruption.
  - Root cause: Bell/Fan/SawBlade vtable functions have different calling conventions
    (different RET N) than PopCylinder's vtable functions. When the game calls these
    overridden vtable functions on a PopCylinder object, the stack gets corrupted.
  - Same pattern as v53f: "NEVER override a vtable on an object allocated with a
    different size/constructor than the vtable's expected struct."
  - Fix: Removed all vtable overrides. Bell/Fan/SawBlade now use PopCylinder's
    native vtable (static mesh, no native animation/behavior). The mesh swap at
    obj+0x08 gives the correct visual appearance.
- **Also includes all v54 fixes:**
  - if/else-if chain bug fix (path=NULL overrides now work)
  - 8Ball mesh fix (App+0x268 → App+0x248, ball+0x754=1)
  - Bell: PopCylinder + .MESH swap (meshes\Bell)
  - Fan: PopCylinder + .MESH swap (meshes\fanbody)
  - SawBlade: PopCylinder + Level8-Saw.MESHWORLD
- Crash test: 38.6s OK

## v54 — MESH File Hotfix

- **CRITICAL FIX: if/else-if chain bug caused ALL path=NULL overrides to be skipped!**
  - The path determination code uses `if (mesh_path) { path = mesh_path; } else if (ai_type == 15) { path = NULL; }`
  - When mesh_path is non-NULL (which it always is for 8ball, Bell, Fan, etc.),
    the first branch sets `path = mesh_path` and ALL else-if branches are SKIPPED.
  - The `ai_type == 15 → path = NULL` override NEVER RAN!
  - Fix: Added a separate override AFTER the if/else-if chain that forces
    `path = NULL` for all entity types that handle their own mesh loading
    (types 12-16, 22, 28, 30-33, 41, 42).
- **8Ball (type 15): Fixed Static Swirl issue!**
  - Root cause 1: if/else-if bug (above) — path was never set to NULL, so
    the .MESH swap code created a PopCylinder with Swirl mesh instead of
    running case 15 (BadBall_ctor).
  - Root cause 2: Ball_Render reads mesh from `App+0x244[ball+0x754 * 4]`,
    NOT from `ball+0x10`. The old code stored MeshNode at the wrong offset.
  - Fix: Copy 8Ball mesh pointer from `App+0x268` to `App+0x248` (slot 1),
    set `ball+0x754 = 1`. Same pattern as `cEnt_process_custom_tags`.
- **Bell (type 30): Fixed!**
  - Bell_ctor calls `Level_ctor` (no mesh). vtable[1] = Rotator_Update needs vertex data.
  - v54 uses PopCylinder_ctor with `meshes\Bell` .MESH swap + vtable override to 0x004D5330.
- **Fan (type 31): Fixed!**
  - Same Level_ctor issue. Uses `meshes\fanbody` .MESH swap + vtable override to 0x004D5180.
- **SawBlade (type 32): Fixed!**
  - Same Level_ctor issue. Uses `levels\Level8-Saw` .MESHWORLD + vtable override to 0x004D5240.
- **Also includes all v53g-5 fixes:**
  - Fixed critical despawn bug (entities destroyed on No-GRID levels)
  - Fixed Trapdoor/Odd_Lifter mesh leak + NULL guard
  - All 34 constructor addresses verified via Ghidra
- Crash test: 38.6s OK

## v53g-5

- **CRITICAL FIX: Entities despawned immediately on levels without GRID points**
  - The "No GRID points found" branch called `cEnt_despawn_all_rotaters()` which
    calls `vtable[11]` (RemoveAndFree) on ALL custom entities that were just
    spawned by `process_rotaters()`. This destroyed every custom entity on
    levels that have no GRID reference points.
  - Fix: Removed the `cEnt_despawn_all_rotaters()` call from the No-GRID branch.
    Entities are still properly cleaned up on level exit via the normal path.
- **FIX: Trapdoor/Odd_Lifter mesh leak and NULL mesh guard**
  - Trapdoor_ctor (0x438290) and Odd_Lifter_ctor (0x434E60) read mesh from
    `board+0x878+0x594/0x5C8` (App mesh table), NOT from the mesh parameter.
    The spawn function was loading a mesh file that was never used (memory leak).
  - Fix: Set `path=NULL` for types 41 and 42 so no mesh file is loaded.
  - Added safety guard: checks if App mesh table entry is valid before calling
    the constructor. If NULL, the entity is skipped (logged) instead of crashing.
- **Verified via Ghidra decompilation**:
  - All 34 constructor addresses confirmed correct
  - All 24 alloc size constants confirmed correct
  - All 8 Stands_ctor family calling conventions confirmed (v53g-4 fixes hold)
  - Stands_ctor (0x462850) internally calls SpriteAnim_Ctor, AthenaList_Init,
    Timer_Init, and SpatialTree_CloneToLevel — all verified safe
  - Collision offset logic (col_off=0 for all except Rotator 1-6) confirmed
  - Board list additions (0x2578, 0xCD4, 0x8B8, scene+0x1C) verified correct
  - Despawn logic (skip types 30-33) verified correct
- Crash test: 39.7s, no crash

## v53g-4

- **REVERTED v53f crash regression**: use_board_level_as_mesh and visual mesh swap
  caused 11 entities to crash (Rotator, Pendulum, Looper, Gear, Swirl, Flickfloor1/2,
  Flickring, Spinner, Trode, Bonk). All entities now load their own MESHWORLD files
  directly (back to v53e behavior).
- **Fixed leftover 0x95E0**: Per-frame monitoring code had a leftover board+0x95E0
  reference that corrupted the game's update list, causing crashes at 0x452376.
- **Fixed Flag/Flag2 crashes**: FlagWaver_Ctor creates a global renderer, not a
  per-entity object. Changed Flag from type 12 (FlagWaver) to type 14 (Wavy_ctor)
  with Flag.MESHWORLD path, same as Flag2.
- **Wobbly wobble**: Added type 8 (GameLevel) to board+0x8B8 (Scene_Update list)
  so Rotator_Update (vtable[1]) is called per-frame for vertex deformation.
- **Bridge tilt animation**: Changed Bridge from type 16 (custom PopCylinder) to
  type 34 (BreakBridge_ctor, 0x436D70) with Pendulum vtable and Rotator_Update.
  Added to board+0x8B8 for per-frame vertex deformation.
- **_default.MESHWORLD placeholder**: Entities with no real mesh (Bumper, Tarpit,
  Chrome) use levels\_default as placeholder.
- Known issues: .MESH entities (8ball, Bell, Fan, Funball) show as static Swirl
  due to MeshWorld structure mismatch. Needs further investigation.

## v53f

- **CRITICAL FIX: Crash root cause found and fixed**
  - Constructors like ArenaStands_ctor, Rotator_ctor, Looper_ctor, etc. internally call
    Stands_ctor/Level_RenderCtor which dereference SceneObject+0x440 (vertex data)
  - Separately-loaded .MESHWORLD meshes had NULL vertex data at this offset
  - Fix: pass the board's own Level (board+0x8AC) as the mesh parameter
  - Board's Level has fully-loaded vertex data - no more NULL dereference crashes
- **Visual mesh swap after construction**
  - After constructing with board Level, swap obj+0x08 (MeshWorld*) to desired visual mesh
  - This gives correct appearance while keeping valid collision/vtable from board Level
- **Rotator oscillation fixed**
  - Native render checks "if angle > 2.0" and "if angle < -2.0" to reverse direction
  - Fix: clamp angle to [-1.99, 1.99] every frame to prevent reversal
  - Constant rotation now works correctly

## v53e

- All mod functions renamed with `cEnt_` prefix to separate from game's originals
- Cloned all constructors/wrappers - mod now works with copies, not game functions directly
- README pipe alignment fixed
- README restructured: version changelogs moved to this file, README keeps only reference tables

## v53d

- Fixed 8ball/BadBall: now loads 8ball.MESH via cEnt_MeshNode_ctor and stores at ball+0x10
- Fixed BadBall alloc size: 0xC98 (was 0xC70)
- Added cEnt_Bonk_ctor (type 33, 0x438850, 0x1200) - Warm-Up Bonk, self-loads level5-bonk
- Bell/Fan/SawBlade: reverted to PopCylinder (type 0) - their ctors call Level_ctor (no mesh)
  and crash during board update because the vtable update method calls LoadMesh with invalid state

## v53c

- Isolated and cloned behaviors: Chomper, Chrome, Funball, Tarbubble, Waterwheel
- Fixed cEnt_Gear_ctor (9 params, was using 6-param Rotator typedef - would crash!)
- Fixed Looper size (0x1500, was 0x1508)
- Added cEnt_Spinner_Level_ctor (Expert Race "BRIDGE")
- Added Cloudscape entity (Sky Race clouds, cEnt_Sprite_ctor)
- Named cEnt_ wrappers for all Neon Race objects (cEnt_DFloor1-4, cEnt_FlickRing_ctor, cEnt_Trode_ctor)

## v53b

- **Deep Ghidra decompilation** of every `CreateDynamicObjects` function in the game
- Added 7 new constructor types with correct alloc sizes and calling conventions
- Fixed 10 entities that were using the wrong `_ctor`
- NULL mesh paths now use `levels\_default` as placeholder (your `_default.MESHWORLD` file)

### Entities fixed in v53b

| Entity       | Old _ctor   | New _ctor             | Address  | Size   |
| ------------ | ----------- | --------------------- | -------- | ------ |
| Drawbridge   | PopCylinder | cEnt_Glass_Level_ctor | 0x4384A0 | 0x113C |
| Flag         | PopCylinder | cEnt_FlagWaver_Ctor   | 0x46AF30 | 0x8C   |
| Flickfloor1  | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Flickfloor2  | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Flickring    | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Glassbreaker | PopCylinder | cEnt_Secret_ctor      | 0x43DFB0 | 0x10EC |
| Judge        | PopCylinder | cEnt_Gear_Level_ctor  | 0x43A150 | 0x1100 |
| Sign         | PopCylinder | cEnt_Sign_ctor        | 0x443B90 | 0x10FC |
| Trode        | PopCylinder | cEnt_ArenaStands_ctor | 0x43E450 | 0x1104 |
| Wobbly       | PopCylinder | cEnt_GameLevel_ctor   | 0x4351F0 | 0x1524 |
