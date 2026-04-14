/*
 * Hamsterball Object Factory System
 * 
 * CreateLevelObjects (0x4121D0) is the main factory dispatcher.
 * Called per named object in the level mesh, uses __strnicmp to match
 * object name prefixes and instantiates the appropriate game object.
 *
 * All created objects are appended to Scene+0x2578 (the active object list).
 * Many objects also get registered to type-specific lists for quick lookup.
 *
 * Object types and their constructors:
 *
 * BRIDGE (prefix match, 6 chars):
 *   - Sets bridge collision mesh reference from Scene+0x436C
 *   - Updates bridge position from param_4 (transform matrix at +4/8/C)
 *   - If no "(NOCOLLIDE)" in name, enables collision via Scene+0x4370
 *   - No new object created, just configures existing bridge mesh
 *
 * TIPPER (prefix match, 6 chars) - if App+0x23C != 0:
 *   - Tipper_ctor(0x1104 bytes) with visual mesh from Scene+0x4394
 *   - Position from param_4+4/8/C, rotation from param_4+10/14/18
 *   - TipperVisual_ctor(0x10D0 bytes) with mesh from Scene+0x4398
 *   - TipperVisual_Attach links visual to physics
 *   - Registered: Scene+0x2578
 *
 * BONK (prefix match, 4 chars) - if App+0x23C != 0:
 *   - Bonk_ctor(0x1200 bytes) with position from param_4
 *   - Registered: Scene+0x2578
 *   - Also stored at Scene+0x540C (the level's bonk reference)
 *
 * BBRIDGE1 (prefix match, 8 chars):
 *   - BreakBridge_ctor(0x1100 bytes) with pos from param_4
 *   - Uses mesh reference from Scene+0x5410
 *   - Registered: Scene+0x2578
 *   - Stored at Scene+0x5418
 *
 * BBRIDGE2 (prefix match, 8 chars):
 *   - BreakBridge_ctor(0x1100 bytes) with pos from param_4
 *   - Uses mesh reference from Scene+0x5414
 *   - Registered: Scene+0x2578
 *   - Stored at Scene+0x541C
 *
 * POPCYLINDER (prefix match, 11 chars):
 *   - PopCylinder_ctor(0x10E8 bytes) with pos from param_4
 *   - Uses mesh reference from Scene+0x5420
 *   - Registered: Scene+0x2578, Scene+0x5428
 *
 * BLOCKDAWG1 (prefix match, 10 chars) - if App+0x23C != 0:
 *   - Blockdawg_ctor(0x1154 bytes) with pos from param_4
 *   - Finds path via Level_FindObjectByName("DAWGPATH1")
 *   - Uses mesh from Scene+0x5840
 *   - Registered: Scene+0x2578
 *
 * BLOCKDAWG2 (prefix match, 10 chars) - if App+0x23C != 0:
 *   - Blockdawg_ctor(0x1154 bytes) with pos from param_4
 *   - Finds path via Level_FindObjectByName("DAWGPATH2")
 *   - Uses mesh from Scene+0x5844
 *   - Sets flag at obj+0x1152 = 1 (variant 2)
 *   - Registered: Scene+0x2578
 *
 * CATAPULT (prefix match, 8 chars):
 *   - Catapult_ctor(0x1108 bytes) with mesh from Scene+0x5848
 *   - Sets flag at obj+0x440*4 = 1 (active)
 *   - Position from param_4+4/8/C
 *   - Registered: Scene+0x2578, Scene+0x584C
 *
 * GLUEBIE (prefix match, 7 chars) - if App+0x23C != 0:
 *   - Gluebie_ctor(0x110C bytes) with mesh from Scene+0x607C
 *   - Position from param_4+4/8/C
 *   - Registered: Scene+0x6080, Scene+0x2578
 *
 * Scene object list offsets (for quick lookup by type):
 *   +0x2578  active_objects (all game objects)
 *   +0x436C  bridge_mesh
 *   +0x540C  bonk_ref
 *   +0x5410  breakbridge1_mesh, +0x5418 breakbridge1_obj
 *   +0x5414  breakbridge2_mesh, +0x541C breakbridge2_obj
 *   +0x5420  popcylinder_mesh, +0x5428 popcylinder_list
 *   +0x5840  blockdawg1_mesh, +0x5844 blockdawg2_mesh
 *   +0x5848  catapult_mesh, +0x584C catapult_list
 *   +0x607C  gluebie_mesh, +0x6080 gluebie_list
 */

/*
 * BoardLevel3_ctor (0x41D060) - "Dizzy Race" level
 * 
 * Demonstrates the board construction pattern:
 *   1. Board_ctor(this, param_1) - base class init
 *   2. Set vtable to PTR_BoardLevel3_Dizzy_dtor_004d0890
 *   3. Init AthenaLists at +0x4378 and +0x4790
 *   4. Set title "Board (Dizzy)", race_name "DIZZY RACE"
 *   5. Load sub-levels as MeshWorld + CollisionLevel pairs:
 *      - Level3-Tipper -> +0x436C / +0x4370 collision
 *      - Level3-WaterWheel -> +0x4BA8 / +0x4BAC collision
 *      - Level3-Swirl -> +0x4BC4 / +0x4BC8 collision
 *      - Level3-Gluebie -> +0x4374 (mesh only, no separate collision)
 *   6. Allocate sound channel via Sound_GetNextChannel
 *   7. Init physics: gravity = (0, 1.0, 0), Matrix_Identity
 *   8. LoadRaceData(this, "DIZZYRACE")
 *   9. Set display_string = "Dizzy!"
 *
 * Each board level follows this pattern with different sub-meshes
 * and collision levels. The board number maps to the level index.
 */

/*
 * App_StartMPRace (0x428B70) - Multiplayer race entry
 *
 * Flow:
 *   1. Clear single-player flags (+0x237, +0x236 = 0)
 *   2. App_StartRace (base race init)
 *   3. Set MP flags: +0x717=1, +0x7B7=1, +0x5D7=0, +0x677=1
 *   4. If is_client (+0x234): +0x677=0
 *   5. Reset D3D cull mode
 *   6. Sync player slots (+0x27C->+0x280, +0x288->+0x28C)
 *   7. Set race_active flag +0x23C=1
 *   8. Create PlayerProfile (0x98 bytes)
 *   9. Set profile.is_host = 1 (+0x10)
 *   10. Tournament_AdvanceRace(profile, 0)
 *   11. Cleanup old callback timers (+0x90C, +0x910)
 */