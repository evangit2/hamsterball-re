# v54d User Feedback — MAKYUNI

## Emoji Legend
- ❌ = Crashes the level
- ⬛ = Not tested yet
- 🟫 = Static Swirl (mesh not loading)
- 🟧 = Early WIP
- 🟨 = WIP - Needs Testing
- 🟩 = Almost done!
- ✅ = DONE!

## Entity Status Table

| Status | Entity | Issue |
|--------|--------|-------|
| ❌ | 8ball | Static swirl + crashes at 0x46186E (Level_ctor) |
| 🔍 | BBridge | Shows up, needs testing. No collision. |
| 🟫 | Bell | Static swirl |
| 🔍 | Blockdawg | Shows up, needs testing |
| 🟧 | Bonk | Doesn't show up. E:CALLHAMMER doesn't work |
| ❌ | Bridge | Crashes at 0x478EDD (MeshArchive_ctor) |
| 🟨 | Bridgeslam | Shows up, no collision |
| 🟨 | Bumper | Shows up, no collision |
| 🟨 | Catapult | Shows up, no collision |
| 🟫 | Chomper | Static swirl + crashes at 0x478EDD |
| 🟧 | Chrome | Loads _default.MESHWORLD, should be Sphere.MESH. No collision |
| ❌ | Cloudscape | Crashes at 0x456FB3 (Gadget_AddSpriteSlot) |
| ❌ | Drawbridge | Crashes at 0x474ABE (LoaderGadget_Tick) |
| 🟧 | Droplifter | Not showing up anymore |
| 🟫 | Fan | Static swirl |
| ❌ | Flag | Crashes at 0x4713C0 (MeshWorld_Parse) |
| ❌ | Flag2 | Crashes at 0x4713C0 (MeshWorld_Parse) |
| ✅ | Flickfloor1 | Works! |
| ✅ | Flickfloor2 | Works! |
| 🟩 | Flickring | Works, missing BUMPER function |
| 🟫 | Funball | Static swirl |
| ❌ | Gear | Crashes at 0x436690 |
| 🟫 | Glassbreaker | Static swirl |
| 🟨 | Gluebie | Shows up, missing Gluebie behaviour |
| 🟫 | Judge | Static swirl |
| ✅ | Lifter | Works! |
| ❌ | Looper | Crashes at 0x439FE9 (FUN_00439bb0) |
| 🟨 | Mace | Shows up, missing behaviour. Needs E:MACETRIGGER |
| 🟫 | Mag | Static swirl |
| 🟨 | Mousetrap | Shows up, missing behaviour. Needs N:Mousetrap |
| 🟨 | Neonplatform | Shows up, missing behaviour |
| ✅ | Pendulum | Works! |
| 🟨 | Popcylinder | Shows up, missing behaviour. Needs E:PEGS, E:NOPEGS |
| 🟩 | Rotator | Works nicely. Should use _ctor 0x4366F0 and update 0x0043D8C0 |
| 🟨 | Saw | Shows up, missing behaviour. Needs C:SawPath |
| 🟫 | Sawblade | Static swirl |
| 🟩 | Sign | Works, missing "Sign-BewareTheTar" texture |
| 🟨 | Speedcylinder | Shows up, has idle animation. Missing behaviour |
| ❌ | Spinner | Crashes at 0x46186E (Level_ctor) |
| 🟩 | Swirl | Works! Missing function from 0x4D5518 |
| 🟫 | Tarbubble | Static swirl |
| 🟨 | Tarpit | Shows up, missing behaviour. Treat as N:TARPIT |
| 🟨 | Timebutton | Shows up, missing behaviour and collision |
| 🟩 | Tipper | Works! Missing collision |
| 🟧 | Trapdoor | Not showing up anymore |
| ✅ | Trode | Works perfectly! |
| 🟨 | Waterwheel | Shows up, missing behaviour and collision |
| 🟨 | Wavy | Shows up, missing behaviour and collision |
| 🟨 | Windmill | Shows up, missing behaviour and collision |
| ❌ | Wobbly | Crashes at 0x478EDD (MeshArchive_ctor) |

## Crash Addresses Decoded

| Entity | Crash Address | Function | Operation |
|--------|--------------|----------|-----------|
| 8Ball | 0x0046186E | Level_ctor | Background |
| Bridge | 0x00478EDD | MeshArchive_ctor | Draw |
| Chomper | 0x00478EDD | MeshArchive_ctor | Draw |
| Cloudscape | 0x00456FB3 | Gadget_AddSpriteSlot | Background |
| Drawbridge | 0x00474ABE | LoaderGadget_Tick | Draw |
| Flag | 0x004713C0 | MeshWorld_Parse | Draw |
| Flag2 | 0x004713C0 | MeshWorld_Parse | Draw |
| Gear | 0x00436690 | (unknown) | Background |
| Looper | 0x00439FE9 | FUN_00439bb0 | Update |
| Spinner | 0x0046186E | Level_ctor | Draw |
| Wobbly | 0x00478EDD | MeshArchive_ctor | Update |

## Summary
- ✅ Working: 5 (Flickfloor1, Flickfloor2, Lifter, Pendulum, Trode)
- 🟩 Almost: 5 (Flickring, Rotator, Sign, Swirl, Tipper)
- 🟨 WIP: 16 (various missing behavior/collision)
- 🟫 Static Swirl: 10 (8ball, Bell, Chomper, Fan, Funball, Glassbreaker, Judge, Mag, Sawblade, Tarbubble)
- ❌ Crashes: 11 (8ball, Bridge, Chomper, Cloudscape, Drawbridge, Flag, Flag2, Gear, Looper, Spinner, Wobbly)
