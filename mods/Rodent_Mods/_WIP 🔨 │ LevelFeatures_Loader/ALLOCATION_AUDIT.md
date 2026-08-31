# LevelFeatures_Loader — Allocation Audit (for dynamic heap)

## Ext heap (per-board, HeapAlloc at board creation)
Current EXT_SIZE = 0xC000-0x6500 = 23296 B (0x5B00). One ext block per board, tracked in g_extMap.

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
WarmUp+bumpers ≈ 0x3350 + 0x410 ≈ 13KB vs current 23KB waste. Master+all ≈ 21KB (matches current).

## Instance heap (per-object g_operatorNew, dynamic at spawn, NOT in ext)

| S1 object | g_operatorNew | Count | Total | Notes |
|---|---|---|---|---|
| Tipper (+Visual) | 0x1104 + 0x10D0 | 1-3 | 8-13KB | Tipper_ctor + render |
| Gluebie | 0x110C | 1 | 4364 | |
| Catapult | 0x1108 | 1 | 4360 | |
| Mace | 0x110C | 1 | 4364 | |
| Drawbridge | 0x113C | 1 | 4412 | |
| Windmill | 0x10D0 | 1 | 4304 | + WM_RENDER ext slot |
| Trapdoor | 0x10F8 | 1 | 4344 | +2 Athena sub-lists |
| Chomper | 0x10D0 | 1 | 4304 | |
| Turret | 0x10D0×2 | 1 | 8608 | mesh+render |
| Bonk (hammer) | 0x1200 | 1 | 4608 | largest fixed |
| Fan | 0x1188 | 1 | 4488 | |
| SawBlade1/2 | 0x111C ×2 | 2 | 8760 | |
| SawBridge (spinner) | 0x10D0 | 1 | 4304 | |
| Bridge | 0x10D0 | 1 | 4304 | +BBOARD lists |
| Judge | 0x1100 | 1 | 4352 | |
| Bell | 0x10E8 | 1 | 4328 | |
| Lifter/SpeedCylinder | 0x10F4 ×N | N | 4340×N | Up has many |
| Dropper (Odd) | 0x10FC | 1 | 4348 | |
| Spinny | 0x1508 | 1 | 5384 | |
| Saw + Saw2 (Toob) | 0x1100+0x1118 | 2 | 8728 | |
| Fallout | 0x10E8 | 1 | 4328 | |
| Blockdawg1/2 | 0x1154 ×2 | 2 | 8872 | |
| Wobbly1-7 | 0x1524 ×7 | 7 | 37884 | worst: 0x724C |
| Wavy1 | 0x1AE7C | 1 | **110204** | **outlier — dominates Master** |
| NeonPlatform | 0x10EC | 1 | 4332 | |
| DFloor1-4 | 0x1104 ×4 | 4 | 17424 | |
| Trode | 0x1104 | 1 | 4356 | |
| PopCylinder | 0x10F4 ×2 | 2 | 8680 | |
| Popdoor (Sky) | 0x10F4 | 1 | 4340 | |
| SpeedCylinder | 0x150C | 1 | 5388 | |
| TimeButton | 0x10E8 | 1 | 4328 | |
| Looper | 0x1500 | 1 | 5376 | |
| Gear/BigGear | 0x1514 ×2 | 2 | 10792 | |
| Rotator/Pendulum | 0x1508/1504 | 1-2 | 5-10KB | |
| BBridge1/2 | 0x1100 ×2 | 2 | 8704 | |
| TarBubble | 0x1C each | many | 28×N | short-lived |
| MeshWorld | 0x10D0 | per file | 4304 | Level_MeshWorldCtor |
| Bumper slot | 0x418 ×8 | 8 | 8384 | inside EHVECTOR ext |

Worst vanilla: Master with Wavy = 110KB + Wobbly + others ≈ 160KB instance + 23KB ext ≈ 183KB.
WarmUp+bumpers ≈ 13KB ext + 8KB instance ≈ 21KB (vs 23KB fixed waste today).

