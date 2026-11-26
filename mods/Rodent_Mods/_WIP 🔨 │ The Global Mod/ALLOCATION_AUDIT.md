# LevelFeatures_Loader — Allocation Audit (for dynamic heap)
Ground truth verified 2026-08-31 against disk: `Level*.MESHWORLD` S1 strings + `Level*-*.MESHWORLD` sub-meshes in `/drive_c/hamsterball_test/Levels/`. S1 names marked "synthetic" are handler aliases with hardcoded mesh paths, not vanilla S1.

## Ext heap (per-board, HeapAlloc at board creation)
Current EXT_SIZE = 0xC000-0x6500 = 23296 B (0x5B00). One ext block per board, tracked in g_extMap. Clean — no physical overlaps after v13h clobber-fix.

### Fixed region (always needed)
| Object | Ext offset | Bytes | Notes |
|---|---|---|---|
| ehVector (bumpers) | 0x6500 | 8384 (0x20C0) | 8 × 0x418 slots — bumper physics |
| Bumper lit flags | 0x85C0 | 32 (0x020) | 8 × 4B int |
| MESH_0..15 generic | 0x85E0 | 64 (0x040) | 16 × 4B mesh ptrs (waterwheel, swirl, etc.) |
| Working data 0x8620 | 0x8620 | 160 (0x0A0) | BONK_STORE, BELL/SAW/BRIDGE/WINDMILL/BITE/NEON/GLASS/PEGS/POPCYL/BLOCKDAWG (40 slots) |
| Dedicated1 (Tipper..Rot) | 0x86C0 | 60 (0x03C) | Tipper/Spinny/Saw/Fallout/Gluebie/Looper/Gear/BigGear meshes + 6 floats rot |
| Sky popcyl array | 0x8700 | 64 (0x040) | 16×4B (Sky only, else 0) |
| Sky timer | 0x8740 | 4 (0x004) | int (Sky only) |
| Render glass/tower/sky | 0xA880 | 80 (0x050) | Glass smasher xyz + tower + sky |
| Collision count+names | 0xA8D0 | 4100 (0x1004) | int + 128×32 char — byte-scanned S1 N:/E: |
| Collision flags | 0xB950 | 64 (0x040) | 62B fast-path |
| Dedicated2 (B8D8) | 0xB8D8 | 116 (0x074) | Catapult/Mace/Drawbridge/Bonk/Windmill/Chomper/Turret/SawBlade/Blockdawg/Lifter/Rotator/DFloor/Wobbly/PopCyl/BBridge |
| Clobber-fix tail | 0xB990 | 76 (0x04C) | SAW2_TOOB, FALLOUT, BRIDGE_PIVOT×3, BB_FLAG/COUNTER/TOTAL/LAST/POS_TABLE(36B), WM_RENDER |
| **Subtotal fixed** | | **13136 (0x3350)** | |

### Variable region (depends on S1 scan)
| Object | Offset | Bytes | Condition |
|---|---|---|---|
| AthenaLists 0..7 | 0x8800 | 8320 (0x2080) | 8×0x410 — BULK. Can be trimmed to S1 count |
| **Total with all lists** | | **21524 (0x5414)** | 1772B spare in 0xC000, 0xBC gap at 0x8744, 0x624 tail free at 0xB9DC |

For dynamic heap: totalExt = fixed(≈13KB without lists) + nLists×0x410 + nCollisions×32 + tail.
WarmUp+bumpers ≈ 0x3350 + 0x410 ≈ 13KB vs current 23KB waste. Level7 Wobbly (worst vanilla, with Wavy1) ≈ 21KB ext + 148KB instance ≈ 169KB.

## Instance heap (per-object g_operatorNew, dynamic at spawn, NOT in ext)

| S1 object | g_operatorNew | Count | Total | Notes | Vanilla level |
|---|---|---|---|---|---|
| Tipper (+Visual) | 0x1104 + 0x10D0 | 1-3 | 8-13KB | Tipper_ctor + Level_RenderCtor | Level3 Dizzy (synthetic S1, sub-mesh Level3-Tipper) |
| Gluebie | 0x110C | 1 | 4364 | | Level3 Dizzy (Level3-Gluebie) |
| WaterWheel | 0x10D0 | 1 | 4304 | meshWorld only | Level3 Dizzy (Level3-WaterWheel) |
| Swirl | 0x1110 | 1 | 4360 | zone mem | Level3 Dizzy (S1 SWIRL) |
| Catapult | 0x1108 | 1 | 4360 | | Level4 Tower (Level4-Catapult) |
| Mace | 0x110C | 1 | 4364 | | Level4 Tower (S1 MACE) |
| Drawbridge | 0x113C | 1 | 4412 | | Level4 Tower (S1 DRAWBRIDGE) |
| Windmill | 0x10D0 | 1 | 4304 | + WM_RENDER ext | Level4 Tower (S1 WINDMILL) |
| Trapdoor | 0x10F8 | 1 | 4344 | +2 sub-lists | Level4 Tower (S1 TRAPDOOR) |
| Chomper | 0x10D0 | 1 | 4304 | | Level4 Tower (S1 CHOMPER) |
| Turret | 0x10D0×2 | 1 | 8608 | mesh+render | Level4 Tower (S1 TURRET) |
| Bonk (hammer) | 0x1200 | 1 | 4608 | largest fixed | Level5 Expert (S1 BONK) |
| Fan | 0x1188 | 1 | 4488 | | Level5 Expert (S1 FAN) |
| SawBlade1/2 | 0x111C ×2 | 2 | 8760 | | Level5 Expert (S1 SAWBLADE) |
| SawBridge (spinner) | 0x10D0 | 1 | 4304 | Spinner_Level_ctor | Level5 Expert (synthetic SAWBRIDGE) |
| Bridge (drawbridge) | 0x10D0 | 1 | 4304 | +BBOARD lists | Level10 Master (S1 BRIDGE) + Intermediate |
| Judge | 0x1100 | 1 | 4352 | | Level5 Expert (S1 JUDGE) |
| Bell | 0x10E8 | 1 | 4328 | | Level5 Expert (S1 BELL) |
| Lifter/SpeedCyl | 0x10F4 ×N | N | 4340×N | Up | Level6 Up (S1 LIFTER) |
| Dropper (Odd) | 0x10FC | 1 | 4348 | Odd_Lifter_ctor | Arena-Odd (synthetic DROPPER) |
| Spinny | 0x1508 | 1 | 5384 | | Level8 Toob (S1 SPINNY) |
| Saw + Saw2 | 0x1100+0x1118 | 2 | 8728 | | Level8 Toob (S1 SAW + Level8-Saw) |
| Fallout | 0x10E8 | 1 | 4328 | | Level8 Toob (S1 FALLOUT1) |
| Blockdawg1/2 | 0x1154 ×2 | 2 | 8872 | | Level10 Master (S1 BLOCKDAWG) + Toob variant |
| Wobbly1-7 | 0x1524 ×7 | 7 | 37884 | worst 0x724C | **Level7 Wobbly** (S1 WOBBLY1-7) |
| Wavy1 | 0x1AE7C | 1 | **110204** | outlier — 110KB | **Level7 Wobbly** (S1 WAVY1, sub-mesh Level7-Wavy1) — NOT Master |
| NeonPlatform | 0x10EC | 1 | 4332 | | LevelDark Neon (S1 NEONPLATFORM) |
| DFloor1-4 | 0x1104 ×4 | 4 | 17424 | | LevelDark Neon (S1 DFLOOR1-4) |
| Trode | 0x1104 | 1 | 4356 | | LevelDark Neon (S1 TRODE) |
| PopCylinder | 0x10F4 ×2 | 2 | 8680 | | Level9 Sky (S1 POPCYLINDER1/2) |
| Popdoor (Sky) | 0x10F4 | 1 | 4340 | Rotator_ctor | Level9 Sky (synthetic POPDOOR) |
| SpeedCylinder | 0x150C | 1 | 5388 | | LevelUp (synthetic SPEEDCYLINDER) |
| TimeButton | 0x10E8 | 1 | 4328 | | LevelImpossible (synthetic TIMEBUTTON) |
| Looper | 0x1500 | 1 | 5376 | | LevelImpossible (S1 LOOPER) |
| Gear/BigGear | 0x1514 ×2 | 2 | 10792 | | LevelImpossible (S1 GEAR/BIGGEAR) |
| Rotator/Pendulum | 0x1508/1504 | 1-2 | 5-10KB | | LevelImpossible (S1 ROTATOR/PENDULUM) |
| BBridge1/2 | 0x1100 ×2 | 2 | 8704 | | Level10 Master (S1 BBRIDGE1/2) |
| TarBubble | 0x1C each | many | 28×N | short-lived | Level3 Dizzy (TarBubble_ctor) |
| MeshWorld | 0x10D0 | per file | 4304 | Level_MeshWorldCtor | any level |
| Bumper slot | 0x418 ×8 | 8 | 8384 | inside EHVECTOR ext | any with N:BUMPER |

Worst vanilla is **Level7 Wobbly** (≈21KB ext + ≈148KB instance ≈169KB: 110KB Wavy1 + 38KB Wobblies + rest), not Master. Master (Level10) is ~21KB ext + ~30KB instance without Wavy.
WarmUp+bumpers ≈13KB ext + 8KB instance ≈21KB (vs 23KB fixed waste today).

Corrections from 2026-08-31 re-audit:
- WAVY1 is Level7 Wobbly, not Master (verified: Level7.MESHWORLD S1 contains WAVY1; Level7-Wavy1.MESHWORLD exists on disk; Level10.MESHWORLD has no WAVY).
- TIPPER/WATERWHEEL/GLUEBIE/CATAPULT are synthetic handler names (hardcoded mesh paths like Level3-Tipper), not vanilla S1 strings — Level3.MESHWORLD only contains SWIRL at the S1 level.
- Existing typo "Master with Wavy" fixed; added Vanilla level column verified against disk.
