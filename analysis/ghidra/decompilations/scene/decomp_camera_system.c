// =============================================================================
// Camera System — CameraLookAt (0x413280)
// =============================================================================
//
// Camera initialization for arena/rumble levels. Sets up the camera
// system for a Board/Scene, loading level geometry and finding the
// CAMERALOOKAT target position.
//
// Address: 0x413280
// Calling convention: __fastcall (ECX = this = Scene/Board)
//
// =============================================================================
// LOGIC
// =============================================================================
//
// 1. Load "levels\\arena-spawnplatform" as MeshWorld (0x10D0 bytes)
//    - MeshWorld_ctor(graphics, "levels\\arena-spawnplatform")
//    - Store at this[0x10E3] (spawn_platform_mesh)
//
// 2. Load "levels\\arena-stands" as MeshWorld (0x10D0 bytes)
//    - MeshWorld_ctor(graphics, "levels\\arena-stands")
//    - Store at this[0x10E4] (stands_mesh)
//
// 3. Initialize Scene
//    - vtable[0x90](spawn_platform_mesh) — add to scene
//    - vtable[0x90](stands_mesh) — add to scene
//    - Level_InitScene(this) — set up projection, fog, find CAMERALOCUS
//
// 4. Find Camera Target
//    - Lookup "CAMERALOOKAT" in scene hash table (this[0x22B])
//    - Store target position:
//      this[0x10DE] = camera_target_X
//      this[0x10DF] = camera_target_Y
//      this[0x10E0] = camera_target_Z
//    - Initialize "current target" to same:
//      this[0x10DB] = camera_target_X (current_X)
//      this[0x10DC] = camera_target_Y (current_Y)
//      this[0x10DD] = camera_target_Z (current_Z)
//
// 5. Set Camera Parameters
//    - this[0xA6F] = 45.0 (0x42340000) — camera distance/zoom
//    - this[0xA70] = 800.0 (0x44480000) — camera height offset
//    - this[0x10E1] = 800.0 — camera max height
//    - this[0x10E2] = 1 — camera initialized flag
//
// 6. Render callback
//    - vtable[0x54]() — post-camera setup render callback
//
// =============================================================================
// KEY CAMERA OFFSETS (in Scene/Board struct)
// =============================================================================
// this[0x10E3] = spawn_platform_mesh (MeshWorld*)
// this[0x10E4] = stands_mesh (MeshWorld*)
// this[0x22B]  = scene_hash_table (AthenaHashTable* for named objects)
// this[0x10DE] = camera_target_X (from CAMERALOOKAT)
// this[0x10DF] = camera_target_Y
// this[0x10E0] = camera_target_Z
// this[0x10DB] = camera_current_X (lerps toward target)
// this[0x10DC] = camera_current_Y
// this[0x10DD] = camera_current_Z
// this[0xA6F]  = camera_distance (45.0 default)
// this[0xA70]  = camera_height (800.0 default)
// this[0x10E1] = camera_max_height (800.0)
// this[0x10E2] = camera_initialized (1 = active)
//
// =============================================================================
// CAMERA SYSTEM OVERVIEW
// =============================================================================
//
// The camera sits at an offset above and behind the CAMERALOOKAT position.
// During gameplay (Ball_Update), the camera is updated in two cases:
//
// 1. BALL-ON-FLOOR event (type 5, depth > 0.1):
//    - Scene_SetCamera(scene, ball, FLAG=1)
//    - Graphics_SetViewport at ball position
//    - If ball goes out of bounds: flag for respawn (0xBA = 1)
//
// 2. Ball_Update Phase 17 (Multiplayer Position Sync):
//    - Camera listener position updated from ball display_pos
//    - App+0x178 is SoundDevice, which also has 3D listener data
//    - Listener positions at App+0x854 + index*0xC (3 floats: X, Y, Z)
//
// Camera update appears to be handled by Scene vtable methods:
// - vtable[0x90]: Scene_AddObject — adds mesh to render pipeline
// - vtable[0x54]: Post-camera render callback
// - Scene_SelectCameraProfile (0x40ACA0): selects camera by difficulty (4-15)
//
// For the reimplementation:
// - Camera follows the CAMERALOOKAT position with lerp
// - In rumble/arena mode: fixed camera above arena center
// - In race mode: camera follows ball with distance 45 and height 800
// - Smooth interpolation: current_X/Y/Z lerps toward target_X/Y/Z each frame
// - Out-of-bounds check triggers respawn when ball goes off visible area