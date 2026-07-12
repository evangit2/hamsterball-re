# XRow Mod Suite

A comprehensive collection of **274 Cheat Engine Auto-Assembler (CEA) scripts** for Hamsterball 3.6.C, created by **XRow** and **HDHereIdMe**, with help from **BookwormKevin**.

Each mod is exported as a standalone `bass.dll` proxy that can be dropped into the game directory.

## Overview

| Stat | Count |
|------|-------|
| Total mods | 274 |
| Pure byte-patch DLLs (auto-generated, working) | 137 |
| Complex scripts (code caves, allocs, hotkeys) | 136 |
| Total DLLs compiled | 273 |

## How to Use

### Quick Install
1. Go to the mod folder you want (e.g. `No_dizzy/`)
2. Rename your original `bass.dll` to `bass_real.dll` (only needed once)
3. Copy the mod's `bass.dll` into the game directory
4. Launch Hamsterball

### Important Notes
- **Only enable ONE mod at a time** — multiple bass.dll proxies cannot coexist
- Some mods have persistent effects even after disabling (Size, Speed)
- Complex scripts marked as "Stub" need manual C translation of CEA logic
- The `.CEA` file in each folder is the original Cheat Engine script for reference

## Mod Categories

### Arena Mods (12 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Adjust_arena_time_with_UP_arrow_and_DOWN_arrow/` | "Adjust arena time with UP arrow and DOWN arrow " | 🔧 | 0 |
| `Allow_1_player_in_Arenas/` | "Allow 1 player in Arenas" | ✅ | 3 |
| `bumping_effects_everywhere_(neon_arena)/` | "bumping effects everywhere (neon arena)" | ✅ | 1 |
| `custom_score_increment/` | "custom score increment" | 🔧 | 0 |
| `de-activate_catapults_(tower_race_tower_arena_and_master_rac/` | "de-activate catapults (tower race, tower arena and master race)" | ✅ | 1 |
| `flickring_doesnt_bump_any_balls_(neon_arena)/` | "flickring doesnt bump any balls (neon arena)" | ✅ | 1 |
| `giant_circle_stands_platforms_(Arenas)/` | "giant circle stands platforms (Arenas)" | ✅ | 1 |
| `infinite_arena_timer/` | "infinite arena timer" | 🔧 | 0 |
| `Players_1-4_teleport_back_when_thrown_off_very_far_in_arenas/` | "Players 1-4 teleport back when thrown off very far in arenas" | 🔧 | 0 |
| `spinning_floor_doesnt_move_any_balls_(dizzy_race_dizzy_arena/` | "spinning floor doesn't move any balls (dizzy race, dizzy arena)" | ✅ | 1 |
| `Teleport_Entities_in_the_middle_of_Arenas_Info_--/` | "Teleport Entities in the middle of Arenas                      [] Info -->" | 🔧 | 0 |
| `turrets_disappear_(tower_race_and_tower_arena)/` | "turrets disappear (tower race and tower arena)" | ✅ | 1 |

### Ball Mods (14 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `8balls_fight_each_other/` | "8balls fight each other" | ✅ | 2 |
| `Ball_Collision_Mods_--/` | "Ball Collision Mods                                                       -->" | 🔧 | 0 |
| `Bumping_other_balls_launches_them_upwards/` | "Bumping other balls launches them upwards" | 🔧 | 0 |
| `Bumping_other_balls_teleports_them/` | "Bumping other balls teleports them" | 🔧 | 0 |
| `Disable_badball_on_Goal_crashing/` | "Disable badball on Goal crashing" | 🔧 | 0 |
| `Flying_Hamsterball_Info_--/` | "Flying Hamsterball                                                            [] Info -->" | 🔧 | 8 |
| `Increased_shattered_ball_fragment_count_when_ball_breaks/` | "Increased shattered ball fragment count when ball breaks" | 🔧 | 0 |
| `Manually_shatterbreak_ball_entities_Info_--/` | "Manually shatter/break ball entities                             [] Info -->" | 🔧 | 0 |
| `Many_hamsters_come_out_when_ball_breaks/` | "Many hamsters come out when ball breaks" | ✅ | 1 |
| `Motionless_8balls/` | "Motionless 8balls" | ✅ | 6 |
| `Players_1-4_8ball_texture/` | "Players 1-4 8ball texture" | 🔧 | 0 |
| `Players_1-4_funball_texture/` | "Players 1-4 funball texture" | 🔧 | 0 |
| `Slippery_8balls/` | "Slippery 8balls" | ✅ | 6 |
| `vacuum_makes_your_hamsterball_smaller_(up_race)/` | "vacuum makes your hamsterball smaller (up race)" | ✅ | 1 |

### Bumper Speed Mods (6 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Beginner_Arena_bumper_speed/` | "Beginner Arena bumper speed" | 🔧 | 0 |
| `Beginner_Race_bumper_speed/` | "Beginner Race bumper speed" | 🔧 | 0 |
| `bumper_effects_everywhere_(toob_race)/` | "bumper effects everywhere (toob race)" | ✅ | 1 |
| `bumpers_dont_bump_any_balls/` | "bumpers don't bump any balls" | ✅ | 1 |
| `Toob_Arena_bumper_speed/` | "Toob Arena bumper speed" | 🔧 | 0 |
| `Toob_Race_bumper_speed/` | "Toob Race bumper speed" | 🔧 | 0 |

### Camera Mods (18 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Aggressive_camera_rotation_(Dizzy_Warning)/` | "Aggressive camera rotation (Dizzy Warning)" | 🔧 | 0 |
| `Camera_360_Rotation/` | "Camera 360 Rotation" | 🔧 | 0 |
| `Camera_Angle_change_2/` | "Camera Angle change 2" | ✅ | 3 |
| `Camera_Angle_change_3/` | "Camera Angle change 3" | ✅ | 3 |
| `Camera_Angle_change/` | "Camera Angle change" | ✅ | 1 |
| `Camera_FOV_far/` | "Camera FOV far" | ✅ | 1 |
| `Camera_FOV_super_far/` | "Camera FOV super far" | ✅ | 1 |
| `Camera_FOV_very_far/` | "Camera FOV very far" | ✅ | 1 |
| `Double_360_Camera_Rotation_(Cinematic)/` | "Double 360 Camera Rotation (Cinematic)" | 🔧 | 0 |
| `Flipped_Camera_(2)/` | "Flipped Camera (2?)" | ✅ | 3 |
| `Flipped_Camera/` | "Flipped Camera" | ✅ | 2 |
| `Flying_Hamsterball_with_Focused_Cam_Info_--/` | "Flying Hamsterball with Focused Cam                          [] Info -->" | 🔧 | 9 |
| `Focused_camera/` | "Focused camera" | ✅ | 1 |
| `Follow_camera/` | "Follow camera" | ✅ | 2 |
| `Upside_Down_2.0/` | "Upside Down 2.0" | ✅ | 1 |
| `Upside_Down_Camera/` | "Upside Down Camera" | ✅ | 1 |
| `Zoom_In_Camera/` | "Zoom In Camera" | ✅ | 1 |
| `Zoom_out_360_Camera_Rotation/` | "Zoom out 360 Camera Rotation" | 🔧 | 0 |

### Entity Removal Mods (21 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `remove_8ballsbadballs/` | "remove 8balls/badballs" | ✅ | 1 |
| `remove_blade_brothers_(expert_race)/` | "remove blade brothers (expert race)" | ✅ | 4 |
| `remove_bridge_(intermediate_and_master_race)/` | "remove bridge (intermediate and master race)" | ✅ | 1 |
| `remove_catapults_(tower_race_arena_and_master_race)/` | "remove catapults (tower race, arena and master race)" | ✅ | 1 |
| `remove_chromes_(odd_race)/` | "remove chromes (odd race)" | ✅ | 3 |
| `remove_dawgs_(toob_race_and_master_race)/` | "remove dawgs (toob race and master race)" | ✅ | 3 |
| `remove_fan-storms_(expert_race_crashes_in_expert_arena)/` | "remove fan-storms (expert race, crashes in expert arena)" | ✅ | 1 |
| `remove_lifters_(up_race_and_up_arena)/` | "remove lifters (up race and up arena)" | ✅ | 1 |
| `remove_maces/` | "remove maces " | ✅ | 1 |
| `remove_neon_bridges_(neon_race)/` | "remove neon bridges (neon race)" | ✅ | 4 |
| `remove_speed_cylinders_(up_race)/` | "remove speed cylinders (up race)" | ✅ | 1 |
| `Remove_the_ball_from_hamster/` | "Remove the ball from hamster" | ✅ | 1 |
| `remove_the_circle_platforms_(arenas)/` | "remove the circle platforms (arenas)" | ✅ | 1 |
| `remove_the_draw-bridge_(tower_race)/` | "remove the draw-bridge (tower race)" | ✅ | 2 |
| `remove_the_flickring_(neon_arena)/` | "remove the flickring (neon arena)" | ✅ | 1 |
| `remove_the_trapdoor_(tower_race)/` | "remove the trapdoor (tower race)" | ✅ | 1 |
| `remove_the_waterwheel_(dizzy_race)/` | "remove the waterwheel (dizzy race)" | ✅ | 1 |
| `remove_the_wavy_floor_(wobbly_race)/` | "remove the wavy floor (wobbly race)" | ✅ | 2 |
| `remove_tipper_floors_(dizzy_and_master_race)/` | "remove tipper floors (dizzy and master race)" | ✅ | 1 |
| `remove_trodes_(neon_race)/` | "remove trodes (neon race)" | ✅ | 1 |
| `remove_windmill_(tower_race)/` | "remove windmill (tower race)" | ✅ | 1 |

### Entity Spawn Mods (38 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Auto_spawn_random-sized_8balls_everywhere/` | "Auto spawn random-sized 8balls everywhere" | 🔧 | 0 |
| `despawn_hammer_(expert_race_and_master_race)/` | "despawn hammer (expert race and master race)" | ✅ | 2 |
| `Players_1-4_automatically_respawn_when_being_far_enough/` | "Players 1-4 automatically respawn when being far enough" | 🔧 | 0 |
| `Press_M_to_spawn_mini_8ball/` | "Press M to spawn mini 8ball" | 🔧 | 0 |
| `Press_M_to_spawn_tiny_8ball/` | "Press M to spawn tiny 8ball" | 🔧 | 0 |
| `Press_S_to_spawn_8ball/` | "Press S to spawn 8ball" | 🔧 | 0 |
| `Press_S_to_spawn_BIG_8ball/` | "Press S to spawn BIG 8ball" | 🔧 | 0 |
| `Press_S_to_spawn_Tar_Signs/` | "Press S to spawn Tar Signs" | 🔧 | 0 |
| `Respawn_Players_1-4_Manually_Info_--/` | "Respawn Players 1-4 Manually                                      [] Info -->" | 🔧 | 0 |
| `spawn_bells_everywhere_(expert_race)/` | "spawn bells everywhere (expert race)" | ✅ | 1 |
| `spawn_big_gears_everywhere_(impossible_race)/` | "spawn big gears everywhere (impossible race)" | ✅ | 1 |
| `spawn_catapults_everywhere_(towerrace)/` | "spawn catapults everywhere (towerrace)" | ✅ | 1 |
| `spawn_dfloor1_platforms_everywhere_(neonrace)/` | "spawn dfloor1 platforms everywhere (neonrace) " | ✅ | 1 |
| `spawn_dfloor2_platforms_everywhere_(neonrace)/` | "spawn dfloor2 platforms everywhere (neonrace) " | ✅ | 1 |
| `spawn_dfloor3_platforms_everywhere_(neonrace)/` | "spawn dfloor3 platforms everywhere (neonrace) " | ✅ | 1 |
| `spawn_dfloor4_platforms_everywhere_(neonrace)/` | "spawn dfloor4 platforms everywhere (neonrace) " | ✅ | 1 |
| `spawn_drawbridge_everywhere_(tower_race)/` | "spawn drawbridge everywhere (tower race)" | ✅ | 2 |
| `spawn_fallout_floors_everywhere_(toob_race)/` | "spawn fallout floors everywhere (toob race)" | ✅ | 1 |
| `spawn_fans_everywhere_(expert_race)/` | "spawn fans everywhere (expert race)" | ✅ | 1 |
| `spawn_flags_everywhere_(all_races)/` | "spawn flags everywhere (all races)" | ✅ | 1 |
| `spawn_flickring_(neon_arena)_UNSTABLE/` | "spawn flickring (neon arena) [UNSTABLE]" | ✅ | 1 |
| `spawn_gears_everywhere_(impossible_race)/` | "spawn gears everywhere (impossible race)" | ✅ | 1 |
| `spawn_gluebies_everywhere_(dizzyrace)/` | "spawn gluebies everywhere (dizzyrace) " | ✅ | 1 |
| `spawn_hammy_judges_everywhere_(expert_race)/` | "spawn hammy judges everywhere (expert race)" | ✅ | 1 |
| `spawn_lifters_everywhere_(odd_race)/` | "spawn lifters everywhere (odd race)" | ✅ | 1 |
| `spawn_loopers_everywhere_(impossible_race)/` | "spawn loopers everywhere (impossible race)" | ✅ | 1 |
| `spawn_maces_everywhere_(towerrace)/` | "spawn maces everywhere (towerrace) " | ✅ | 1 |
| `spawn_pendulums_everywhere_(impossible_race)/` | "spawn pendulums everywhere (impossible race)" | ✅ | 1 |
| `spawn_pillars_everywhere_(sky_race)/` | "spawn pillars everywhere (sky race)" | ✅ | 1 |
| `spawn_platforms_everywhere_(Arenas)/` | "spawn platforms everywhere (Arenas)" | ✅ | 1 |
| `spawn_rotating_platforms_everywhere_(impossible_race)/` | "spawn rotating platforms everywhere (impossible race)" | ✅ | 1 |
| `spawn_saw_everywhere_(toob_race)/` | "spawn saw everywhere (toob race)" | ✅ | 1 |
| `spawn_speedcylinders_everywhere_(up_race)/` | "spawn speedcylinders everywhere (up race)" | ✅ | 1 |
| `spawn_time_buttons_everywhere_(up_race)/` | "spawn time buttons everywhere (up race)" | ✅ | 1 |
| `spawn_tipper_floors_everywhere_(dizzy_race)/` | "spawn tipper floors everywhere (dizzy race)" | ✅ | 1 |
| `spawn_trapdoors_everywhere_(tower_race)/` | "spawn trapdoors everywhere (tower race)" | ✅ | 1 |
| `spawn_trodes_everywhere_(neonrace)/` | "spawn trodes everywhere (neonrace) " | ✅ | 1 |
| `Spawnclone_players_1-4_Info_--/` | "Spawn/clone players 1-4                                                 [] Info -->" | 🔧 | 1 |

### Gameplay Mods (9 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Chaos_mode/` | "Chaos mode" | ✅ | 1 |
| `Confusing/` | "Confusing" | ✅ | 1 |
| `Heavy_Chaos_mode/` | "Heavy Chaos mode" | ✅ | 1 |
| `No_dizzy/` | "No dizzy" | ✅ | 1 |
| `No_fall_damage/` | "No fall damage" | ✅ | 8 |
| `stronger_teleporting_effects_while_falling_(dizzy_race)/` | "stronger teleporting effects while falling (dizzy race)" | ✅ | 1 |
| `teleporting_effects_while_falling_(dizzy_race)/` | "teleporting effects while falling (dizzy race)" | ✅ | 1 |
| `Very_Confusing/` | "Very Confusing" | ✅ | 1 |
| `you_can_swim_in_tarpit_(dizzy_race_and_master_race)/` | "you can swim in tarpit (dizzy race and master race)" | ✅ | 1 |

### Global Effect Mods (65 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Global_Controlled_8ball_spawn/` | "Global Controlled 8ball spawn" | 🔧 | 0 |
| `Global_Controlled_Tar_Signs_spawn/` | "Global Controlled Tar Signs spawn" | 🔧 | 0 |
| `Global_spawn_8balls_everywhere/` | "Global spawn 8balls everywhere " | ✅ | 1 |
| `Global_spawn_tar_signs_everywhere/` | "Global spawn tar signs everywhere " | ✅ | 1 |
| `Press_1234_to_spawn_neon_DFloors_1-4_GLOBALLY/` | "Press 1,2,3,4 to spawn neon DFloors 1-4 GLOBALLY" | 🔧 | 0 |
| `Press_1-5_to_spawn_impossible_gears_GLOBALLY/` | "Press 1-5 to spawn impossible gears GLOBALLY" | 🔧 | 0 |
| `Press_1-6_to_spawn_moving_platforms_GLOBALLY/` | "Press 1-6 to spawn moving platforms GLOBALLY" | 🔧 | 0 |
| `Press_1-9_A-F_to_spawn_Arenas_GLOBALLY/` | "Press 1-9, A-F to spawn Arenas GLOBALLY" | 🔧 | 0 |
| `Press_2-7_to_spawn_wobbly_2-7_GLOBALLY/` | "Press 2-7 to spawn wobbly 2-7 GLOBALLY" | 🔧 | 0 |
| `Press_A-S_to_spawn_moving_blockdawgs_GLOBALLY/` | "Press A-S to spawn moving blockdawgs GLOBALLY" | 🔧 | 0 |
| `Press_A-S_to_spawn_secret_platforms_GLOBALLY/` | "Press A-S to spawn secret platforms GLOBALLY" | 🔧 | 0 |
| `Press_A-S_to_spawn_spinning_secret_platforms_GLOBALLY/` | "Press A-S to spawn spinning secret platforms GLOBALLY" | 🔧 | 0 |
| `Press_A-S_to_spawn_tipping_secret_platforms_GLOBALLY/` | "Press A-S to spawn tipping secret platforms GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_8ball_popcylinders_GLOBALLY/` | "Press S to spawn 8ball popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_ball_chromes_GLOBALLY/` | "Press S to spawn ball chromes GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Beginner_Race_GLOBALLY/` | "Press S to spawn Beginner Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_bonk_the_hammer_GLOBALLY/` | "Press S to spawn bonk the hammer GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_bump_popcylinders_GLOBALLY/` | "Press S to spawn bump popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_catapults_GLOBALLY/` | "Press S to spawn catapults GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Dizzy_Race_GLOBALLY/` | "Press S to spawn Dizzy Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_drawbridge_GLOBALLY/` | "Press S to spawn drawbridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_fallout_floors_GLOBALLY/` | "Press S to spawn fallout floors GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_fans_GLOBALLY/` | "Press S to spawn fans GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_flickring_GLOBALLY/` | "Press S to spawn flickring GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_funballs_GLOBALLY/` | "Press S to spawn funballs GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_gluebies_GLOBALLY/` | "Press S to spawn gluebies GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_hamsterball_8ball_GLOBALLY/` | "Press S to spawn hamsterball 8ball GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Intermediate_Race_GLOBALLY/` | "Press S to spawn Intermediate Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_jump_popcylinders_GLOBALLY/` | "Press S to spawn jump popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_lifters_GLOBALLY/` | "Press S to spawn lifters GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_mace_bridge_GLOBALLY/` | "Press S to spawn mace bridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_maces_GLOBALLY/` | "Press S to spawn maces GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Master_Race_GLOBALLY/` | "Press S to spawn Master Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_neon_platforms_GLOBALLY/` | "Press S to spawn neon platforms GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_neon_trodes_GLOBALLY/` | "Press S to spawn neon trodes GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_odd_lifters_GLOBALLY/` | "Press S to spawn odd lifters GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_popcylinders_GLOBALLY/` | "Press S to spawn popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_portal_popcylinders_GLOBALLY/` | "Press S to spawn portal popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_red_bridge_GLOBALLY/` | "Press S to spawn red bridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_sawblade_GLOBALLY/` | "Press S to spawn sawblade GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Sky_Race_GLOBALLY/` | "Press S to spawn Sky Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_speed_popcylinders_GLOBALLY/` | "Press S to spawn speed popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_speedcylinders_GLOBALLY_Info_--/` | "Press S to spawn speedcylinders GLOBALLY                [] Info -->" | 🔧 | 0 |
| `Press_S_to_spawn_spinning_bridge_GLOBALLY/` | "Press S to spawn spinning bridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_spinning_red_bridge_GLOBALLY/` | "Press S to spawn spinning red bridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_spinning_waterwheel_GLOBALLY/` | "Press S to spawn spinning waterwheel GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_spinning_windmills_GLOBALLY/` | "Press S to spawn spinning windmills GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_static_half_trapdoors_GLOBALLY/` | "Press S to spawn static half trapdoors GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_strong_bump_popcylinders_GLOBALLY/` | "Press S to spawn strong bump popcylinders GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_swirl_platform_GLOBALLY/` | "Press S to spawn swirl platform GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_timebuttons_GLOBALLY/` | "Press S to spawn timebuttons GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_tippers_GLOBALLY/` | "Press S to spawn tippers GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_tipping_bridge_GLOBALLY/` | "Press S to spawn tipping bridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_tipping_fallout_floors_GLOBALLY/` | "Press S to spawn tipping fallout floors GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_tipping_red_bridge_GLOBALLY/` | "Press S to spawn tipping red bridge GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_tipping_swirl_platform_GLOBALLY/` | "Press S to spawn tipping swirl platform GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Toob_Race_GLOBALLY/` | "Press S to spawn Toob Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_toob_saw_GLOBALLY/` | "Press S to spawn toob saw GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_toob_spinny_GLOBALLY/` | "Press S to spawn toob spinny GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_Tower_Race_GLOBALLY/` | "Press S to spawn Tower Race GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_weak_balls_GLOBALLY/` | "Press S to spawn weak balls GLOBALLY" | 🔧 | 0 |
| `Press_S_to_spawn_wobbly_floor_GLOBALLY/` | "Press S to spawn wobbly floor GLOBALLY" | 🔧 | 0 |
| `Press_S-A_to_spawn_blockdawgs_GLOBALLY/` | "Press S-A to spawn blockdawgs GLOBALLY" | 🔧 | 0 |
| `Press_S-A_to_spawn_spinning_blockdawgs_GLOBALLY/` | "Press S-A to spawn spinning blockdawgs GLOBALLY" | 🔧 | 0 |
| `Press_S-A_to_spawn_tipping_blockdawgs_GLOBALLY/` | "Press S-A to spawn tipping blockdawgs GLOBALLY" | 🔧 | 0 |

### Glubie Mods (4 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Glubies_huge_size_(respawning_speed_is_fast)/` | "Glubies huge size (respawning speed is fast)" | ✅ | 1 |
| `Glubies_small_size_(respawning_speed_is_slow)/` | "Glubies small size (respawning speed is slow)" | ✅ | 1 |
| `Glubies_tiny_size_(respawning_speed_is_very_slow)/` | "Glubies tiny size (respawning speed is very slow)" | ✅ | 1 |
| `remove_glubies/` | "remove glubies" | ✅ | 1 |

### Gravity Mods (5 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `disable_gravity_on_walls_(odd_race_and_odd_arena)/` | "disable gravity on walls (odd race and odd arena)" | ✅ | 1 |
| `gravity_doesnt_return_after_changing_(odd_race_odd_arena)/` | "gravity doesnt return after changing (odd race, odd arena)" | ✅ | 1 |
| `Gravity_Flip_Cycle_everywhere/` | "Gravity Flip Cycle everywhere" | 🔧 | 0 |
| `Reverse_Gravity_(enable_No_Fall_Damage_for_this)/` | "Reverse Gravity (enable No Fall Damage for this)" | 🔧 | 0 |
| `Zero_Gravity_(enable_No_Fall_Damage_for_this)/` | "Zero Gravity (enable No Fall Damage for this)" | 🔧 | 0 |

### Miscellaneous (38 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `3D_clouds_(sky_race)/` | "3D clouds (sky race)" | ✅ | 1 |
| `auto-Goal_when_going_down_or_going_through_tubespipes/` | "auto-Goal when going down or going through tubes/pipes" | ✅ | 1 |
| `Catapult_random_effect_everywhere/` | "Catapult random effect everywhere" | 🔧 | 0 |
| `Custom_Texts/` | "Custom Texts" | ✅ | 3 |
| `dawg_tower_stack_(toob_race)/` | "dawg tower stack (toob race)" | ✅ | 1 |
| `deactivate_bonus-time_button_(up_race)/` | "deactivate bonus-time button (up race)" | ✅ | 1 |
| `disable_GOAL/` | "disable "GOAL"" | ✅ | 1 |
| `disable_jumping_in_jump_pipes_(odd_race)/` | "disable jumping in jump pipes (odd race)" | ✅ | 1 |
| `disable_shrinking_in_pipe_(odd_race)/` | "disable shrinking in pipe (odd race)" | ✅ | 1 |
| `Disable_size-breaking_collisions/` | "Disable size-breaking collisions" | 🔧 | 0 |
| `Disable_sound_effects/` | "Disable sound effects" | ✅ | 1 |
| `draw-bridge_doesnt_open_(tower_race)/` | "draw-bridge doesn't open (tower race)" | ✅ | 1 |
| `Entities_attract_each_other/` | "Entities attract each other" | 🔧 | 0 |
| `Entities_Jump_Boosts_Info_--/` | "Entities Jump Boosts                                                       [] Info -->" | 🔧 | 0 |
| `Entities_Speed_Boosts_Info_--/` | "Entities Speed Boosts                                                      [] Info -->" | 🔧 | 0 |
| `Entity_Sizes_real-time_change_with_Hotkeys_Info_--/` | "Entity Sizes real-time change with Hotkeys                [] Info -->" | 🔧 | 0 |
| `Entity_Speed_and_Size/` | "Entity Speed and Size " | 🔧 | 0 |
| `Flag_wind_speed_fast/` | "Flag wind speed fast" | ✅ | 1 |
| `Global_breaking_effect/` | "Global breaking effect" | ✅ | 1 |
| `Global_dropping_sound_effect/` | "Global dropping sound effect" | ✅ | 1 |
| `Global_jumping_effect/` | "Global jumping effect" | ✅ | 1 |
| `Global_tarpit_effect/` | "Global tarpit effect " | ✅ | 1 |
| `grow_hamster_after_shrink_(odd_race)/` | "grow hamster after shrink (odd race)" | ✅ | 2 |
| `hammer_doesnt_chase_you_(expert_and_master_race)/` | "hammer doesnt chase you (expert and master race)" | ✅ | 1 |
| `jump-pipe_brings_you_out_of_the_course_(odd_race)/` | "jump-pipe brings you out of the course (odd race)" | ✅ | 2 |
| `magnifier_glass_doesnt_follow_you_(sky_race)/` | "magnifier glass doesnt follow you (sky race)" | ✅ | 1 |
| `magnifier_glass_follows_everywhere_(sky_race)/` | "magnifier glass follows everywhere (sky race)" | ✅ | 1 |
| `misplaced_chomper_(tower_race)/` | "misplaced chomper (tower race)" | ✅ | 1 |
| `Press_G_to_win_instantly/` | "Press G to win instantly" | 🔧 | 0 |
| `shrink_hamster_(odd_race)/` | "shrink hamster (odd race)" | ✅ | 1 |
| `Sizes_roulette_everywhere/` | "Sizes roulette everywhere" | 🔧 | 0 |
| `Speedcylinder_random_effect_everywhere/` | "Speedcylinder random effect everywhere" | 🔧 | 0 |
| `Spinning_random_effect_everywhere/` | "Spinning random effect everywhere" | 🔧 | 0 |
| `teleport_after_falling_(odd_race)/` | "teleport after falling (odd race)" | ✅ | 1 |
| `ten-bonus_glass_dont_break_(glass_race)/` | "ten-bonus glass dont break (glass race)" | ✅ | 2 |
| `Tornado_random_effect_everywhere/` | "Tornado random effect everywhere" | 🔧 | 0 |
| `vacuum_brings_you_in_the_sky_(up_race)/` | "vacuum brings you in the sky (up race)" | ✅ | 1 |
| `vacuum_doesnt_pull_you_up_(up_race)/` | "vacuum doesn't pull you up (up race)" | ✅ | 1 |

### Mouse/Input Mods (3 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `mouse_reversed/` | "mouse reversed" | ✅ | 1 |
| `mouse_sensitivity_higher_than_max/` | "mouse sensitivity higher than max" | ✅ | 1 |
| `mouse_sensitivity_insane/` | "mouse sensitivity insane" | ✅ | 1 |

### Mousetrap Mods (12 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Custom_Mousetrap_XYZ_Force/` | "Custom Mousetrap XYZ Force" | ✅ | 1 |
| `Global_Controlled_Mousetrap_spawn/` | "Global Controlled Mousetrap spawn" | 🔧 | 0 |
| `Global_spawn_mousetraps_everywhere/` | "Global spawn mousetraps everywhere " | ✅ | 1 |
| `Intense_mousetraps_(x6.5_speed)/` | "Intense mousetraps (x6.5 speed)" | ✅ | 1 |
| `Mid-Strong_mousetraps_(x3.75_speed)/` | "Mid-Strong mousetraps (x3.75 speed)" | ✅ | 1 |
| `Press_S_to_spawn_Mousetrap/` | "Press S to spawn Mousetrap" | 🔧 | 0 |
| `remove_mousetraps/` | "remove mousetraps" | ✅ | 1 |
| `Strong_mousetraps_(x2.5_speed)/` | "Strong mousetraps (x2.5 speed)" | ✅ | 1 |
| `Stronger_mousetraps_(x4.5_speed)/` | "Stronger mousetraps (x4.5 speed)" | ✅ | 1 |
| `Torrential_mousetraps_(x10_speed)/` | "Torrential mousetraps (x10 speed)" | ✅ | 1 |
| `Trained_mousetraps_(x1.5_speed)/` | "Trained mousetraps (x1.5 speed)" | ✅ | 1 |
| `Weaker_mousetraps_(x0_speed)/` | "Weaker mousetraps (x0 speed)" | ✅ | 1 |

### Physics Mods (9 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Bouncy_physics_everywhere/` | "Bouncy physics everywhere" | 🔧 | 0 |
| `Dizzy_physics_everywhere/` | "Dizzy physics everywhere" | 🔧 | 0 |
| `Glass_floor_physics_everywhere/` | "Glass floor physics everywhere" | 🔧 | 0 |
| `Ice_physics_(low_friction)_everywhere/` | "Ice physics (low friction) everywhere" | 🔧 | 0 |
| `Magnet_attraction_physics_towards_one_point/` | "Magnet attraction physics towards one point" | 🔧 | 0 |
| `Magnet_repulsion_physics_from_one_point/` | "Magnet repulsion physics from one point" | 🔧 | 0 |
| `Random_Speed_boost_physics_everywhere/` | "Random Speed boost physics everywhere" | 🔧 | 0 |
| `Speed_boost_physics_everywhere/` | "Speed boost physics everywhere" | 🔧 | 0 |
| `TarMud_physics_everywhere/` | "Tar/Mud physics everywhere" | 🔧 | 0 |

### Player Mods (3 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Player_1_becomes_a_magnet/` | "Player 1 becomes a magnet" | 🔧 | 0 |
| `Player_1_repels_entities/` | "Player 1 repels entities" | 🔧 | 0 |
| `Players_1-4_teleport_back_..._but_keep_their_speed_on_telepo/` | "Players 1-4 teleport back ~...~ but keep their speed on teleport" | 🔧 | 0 |

### Visual/Lighting Mods (17 mods)

| Folder | Description | Type | Patches |
|--------|-------------|------|---------|
| `Abnormal_rendering/` | "Abnormal rendering" | ✅ | 1 |
| `Ambient_lighting_color_cycle/` | "Ambient lighting color cycle" | 🔧 | 0 |
| `Background_color_cycle/` | "Background color cycle" | 🔧 | 0 |
| `Dark_background_color/` | "Dark background color" | 🔧 | 0 |
| `Darker_Lighting/` | "Darker Lighting" | 🔧 | 0 |
| `Invisible_Objects/` | "Invisible Objects" | ✅ | 1 |
| `light_is_always_on_(neon_race)/` | "light is always on (neon race)" | ✅ | 1 |
| `light_isnt_brought_back_after_going_off_(neon_race)/` | "light isn't brought back after going off (neon race)" | ✅ | 1 |
| `lights_out_when_you_go_in_tube_(neonrace)/` | "lights out when you go in tube (neonrace)" | ✅ | 1 |
| `Pitch_Black_lighting/` | "Pitch Black lighting" | 🔧 | 0 |
| `Turn_off_lights_with_Blue_ball_lighting/` | "Turn off lights with Blue ball lighting" | 🔧 | 0 |
| `Turn_off_lights_with_Gold_ball_lighting/` | "Turn off lights with Gold ball lighting" | 🔧 | 0 |
| `Turn_off_lights_with_Green_ball_lighting/` | "Turn off lights with Green ball lighting" | 🔧 | 0 |
| `Turn_off_lights_with_Purple_ball_lighting/` | "Turn off lights with Purple ball lighting" | 🔧 | 0 |
| `Turn_off_lights_with_Red_ball_lighting/` | "Turn off lights with Red ball lighting" | 🔧 | 0 |
| `Turn_off_lights_with_Sky_Blue_ball_lighting/` | "Turn off lights with Sky Blue ball lighting" | 🔧 | 0 |
| `Turn_off_lights_with_White_ball_lighting/` | "Turn off lights with White ball lighting" | 🔧 | 0 |

## Build

Each mod folder contains:
- `mod.c` — C source code (auto-generated from CEA)
- `bass.dll` — Compiled DLL proxy (MinGW cross-compiled)
- `build.sh` — Build script
- `script.CEA` or embedded in mod.c — Original CEA script

### Rebuild all DLLs
```bash
cd "mods/XRow Mod Suite"
for dir in */; do
    if [ -f "$dir/mod.c" ]; then
        (cd "$dir" && i686-w64-mingw32-gcc -shared -o bass.dll mod.c -Wl,--enable-stdcall-fixup -luser32)
    fi
done
```

## Credits

- **XRow** — CEA scripts, mod design
- **HDHereIdMe** — CEA scripts, mod design  
- **BookwormKevin** — Script upgrades and assistance
- **Hamsterbot** — DLL auto-generation from CEA table

## Source

Original CT file: `Hamsterball_3.6.C_MOD.CT` (Cheat Engine table, 741KB, 280 CEA scripts)
