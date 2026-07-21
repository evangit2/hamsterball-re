# Hamsterball .MAX Source Files Documentation

## Overview

This folder contains **227 3ds Max source files** (144.1 MB) for the original Hamsterball game. These are the original 3D scene files used by Raptisoft to create the game's levels and meshes.

- **Files:** 227 `.max` files + 3 supporting files (`.3DS`, `.jpg` textures in Rack/)
- **Total size:** 144.1 MB
- **3ds Max versions:** 5.0 and 5.1
- **Total vertices across all files:** 785,386
- **Total faces across all files:** 918,012
- **Computer names found:** CORE2DEV, MAINDEV, RAPTISOFT, GATEWAY
- **User name:** Administrator (all files)

## Folder Structure

```
MAX/
├── Source Levels/          # Level scene files (135 MB)
│   ├── Arenas/             # Arena versions of all 13 race levels
│   ├── Cascade/            # Beginner Race (earlier/alt name: "Cascade Race")
│   ├── Dark/               # Neon Race (internal name: "Dark")
│   ├── Dizzy/              # Dizzy Race + individual object source files
│   ├── Expert/             # Expert Race
│   ├── Glass/               # Glass Race
│   ├── Impossible/          # Impossible Race + individual objects
│   ├── Intermediate/        # Intermediate Race
│   ├── LevelPieces/         # Shared level components (StartPlatform)
│   ├── Master/              # Master Race
│   ├── Spaghetti/           # Odd Race (internal name: "Spaghetti") + Cannon, Cloudscape, etc.
│   ├── Title/               # Title screen 3D objects
│   ├── Toob/                # Toob Race + Icicle, Vacuum, BlockDawg, Saw, etc.
│   ├── Tower/               # Tower Race + Chomper, Catapult, Turret, Mace, etc.
│   ├── Unusual/             # Up Race (internal name: "Unusual") — wait, actually:
│   ├── Up/                   # Up Race
│   ├── Warmup/              # Warm-Up Race
│   └── Wobbly/              # Wobbly Race
├── Source Meshes/           # Character/object mesh source files (11 MB)
│   └── Rack/                # Pool rack model + textures
└── MAX_docs.md             # This file
```

## Internal Level Name Mapping

The .MAX folder names reveal the internal development names for levels:

| .MAX Folder | Game Level |
|-------------|-----------|
| Cascade | Beginner Race (earlier name: "Cascade Race") |
| Dark | Neon Race |
| Spaghetti | Odd Race |
| Unusual | Up Race (Lifter objects shared between Unusual/ and Up/) |

## Notable Source Files

### Character Models (Source Meshes/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Hamster.max | 541 | 662 | Hamster.jpg | Base hamster model, 17 objects, 4 bones (rigged) |
| Hamster-Rigged.max | 541 | 662 | Hamster.jpg | Identical to Hamster.max (same size) |
| Hamster-Rigged-Run1.max | 541 | 662 | Hamster.jpg | Running animation pose 1 |
| Hamster-Rigged-Run2.max | 541 | 662 | Hamster.jpg | Running animation pose 2 |
| Hamster-Rigged-Run3.max | 541 | 662 | Hamster.jpg | Running animation pose 3 |
| Hamster-Rigged-Running.max | 541 | 662 | Hamster.jpg | Another running pose |
| Hamster-Rigged-HIRes.max | 541 | 662 | texture01.jpg | High-res version (same geometry) |
| Hamster-TitlePose.max | 1812 | 1890 | Hamster.jpg | Title screen pose (3x geometry detail) |
| Hamster-Box.max | 6810 | 6888 | texture01.jpg | Boxed/collided version (12x geometry detail) |
| Hamster-Collapsed.max | 5120 | 5116 | (none) | Collapsed mesh (no bones) |
| Hamster-Judge.max | 300 | 298 | Hamster.jpg | Judge stand model |
| Judge.max | 314 | 299 | Hamster.jpg | Separate judge model |
| Mouse.max | 1552 | 1546 | Lakerem.jpg | Mouse character (unused in game) |
| Mouse(Low Res).max | 163 | 255 | Lakerem.jpg | Low-poly version of Mouse |
| Eye.max | 114 | 128 | PurpleEye.png + 6 others | Eye model (unused, orphaned) |

### Skeletal Rigging (Hamster Model)

All Hamster-rigged models share the same 4-bone skeleton:
1. **HeadBone** — head
2. **Upper Body Bone** — torso
3. **Lower Body Bone** — hips
4. **Tail Bone** — tail

Plus additional bones for legs (Right/Left Hindleg 1-3, Right/Left Foreleg 1-3) found in the object list.

### Ball/Mesh Models

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Sphere.max | 96 | 188 | HamsterBall.png, RedBlueChecker.bmp | Standard ball |
| Sphere-4Render.max | 482 | 960 | HamsterBall.png, RedBlueChecker.bmp | High-detail render ball |
| Sphere+Tar.max | 43 | 66 | TarSplotch.png | Ball with tar overlay |
| Sphere+Tar01.max | 43 | 66 | tarsplotch.png | Same, lowercase texture ref |
| Sphere+Dizzy.max | 100 | 190 | Dizzies.png, HamsterBall.png | Ball with dizzy effect |
| ShatteredSphere.max | 110 | 138 | HamsterBall.png | Shattered ball (game over?) |
| Billiard.max | 75 | 146 | 8Ball.png | 8-ball (used for 8-ball enemies) |
| BeeBee.max | 75 | 146 | Chrome.png + 4 others | Chrome ball entity |
| FunBall.max | 75 | 146 | FunBall.png | Fun ball entity |
| GoldenWeasel.max | 13809 | 18452 | Burloak.jpg, Oak1.tga | Golden Weasel (trophy?) |

### Medal Models

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| GoldMedal.max | 936 | 1429 | GerbilDude.png, HamsterDude.png, MouseDude.png, Lakerem.jpg | Gold medal |
| SilverMedal.max | 936 | 1429 | Same as Gold | Silver medal |
| BronzeMedal.max | 936 | 1429 | GerbilDude.png, Lakerem2.jpg | Bronze medal |

### Level Object Models

#### Dizzy Race Objects (Source Levels/Dizzy/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Level3-Swirl.max | 153 | 98 | (11 textures) | SWIRL rotating object |
| Level3-Gluebie.max | 48 | 92 | TarBlot.png | Gluebie (sticky trap) |
| Level3-Tipper.max | (large) | (large) | (6 textures) | Tilting platform |
| Level3-WaterWheel.max | (large) | (large) | (9 textures) | Water wheel |
| Level3-Waterspout.max | (large) | (large) | (6 textures) | Water spout |
| Level3-TarBubble.max | (large) | (large) | (7 textures) | Tar bubble |
| Level3-TwirlPipe.max | 1,191,424B | (largest Dizzy file) | (12 textures) | Twirl pipe |
| Level3-Limit.max | 12619 | 18885 | (11 textures) | Level boundary |

#### Tower Race Objects (Source Levels/Tower/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Level4-Chomper.max | 1224 | 1148 | Wood.png | Chomper entity |
| Level4-Catapult.max | (large) | (large) | (3 textures) | Catapult |
| Level4-Turret.max | 208 | 121 | Wood.png | Turret |
| Level4-Turret-Optimized.max | 144 | 109 | Wood.png | Optimized turret |
| Level4-Mace.max | 314 | 616 | Wood.png | Mace weapon |
| Level4-Mace (FINAL).max | 355 | 383 | (none) | Final version of Mace |
| Level4-Drawbridge.max | 28 | 30 | Wood.png | Drawbridge |
| Level4-Windmill.max | 192 | 118 | Wood.png | Windmill |
| Level4-Trapdoor.max | 24 | 18 | Wood.png | Trapdoor |
| Level4-Trapdoor(Compiled).max | 32 | 18 | Wood.png | Compiled version |
| Level4-Trapdoor(Compiled1).max | 16 | 9 | Wood.png | Compiled v1 |
| Level4-Trapdoor(Compiled2).max | 16 | 9 | Wood.png | Compiled v2 |

#### Impossible Race Objects (Source Levels/Impossible/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Rotator.max | (large) | (large) | (10 textures) | SWIRL rotator |
| Rotator-Ref.max | (large) | (large) | (10 textures) | Reference version |
| Pendulum.max | (large) | (large) | (10 textures) | Pendulum |
| Pendulum-Ref.max | (large) | (large) | (10 textures) | Reference version |
| Gear.max | (large) | (large) | (11 textures) | Gear |
| BigGear.max | (large) | (large) | (11 textures) | Big gear |
| Loopy.max | (large) | (large) | (10 textures) | Looper |
| Loopy-Short.max | (large) | (large) | (10 textures) | Short version |
| Loopy-Clear.max | (large) | (large) | (10 textures) | Clear version |
| BentRail.max | (large) | (large) | (10 textures) | Bent rail |
| Pipe.max | (large) | (large) | (10 textures) | Pipe |

#### Toob Race Objects (Source Levels/Toob/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Icicle.max | (large) | (large) | Ice.png, Icicles.png | Icicle obstacle |
| Vaccuum.max | (large) | (large) | (5 textures) | Vacuum (note: misspelled "Vaccuum") |
| BlockDawg1.max | (large) | (large) | dawggie.png | BlockDawg obstacle |
| BlockDawg1Shoe.max | (large) | (large) | (7 textures) | BlockDawg shoe |
| BlockDawgShadow.max | (large) | (large) | (7 textures) | BlockDawg shadow mesh |
| Fallout.max | (large) | (large) | (6 textures) | Fallout |
| Saw.max | (large) | (large) | saw.png, sawblade.png, sawface.png | Saw blade |
| Toob-Spinner.max | (large) | (large) | (7 textures) | Toob spinner |

#### Odd Race Objects (Source Levels/Spaghetti/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Cannon.max | (large) | (large) | (none) | Cannon entity |
| Cannon-JustCannon.max | (large) | (large) | (none) | Cannon only (no balloon) |
| Cannon-JustBalloon.max | (large) | (large) | (none) | Balloon only (no cannon) |
| Cloudscape.max | (large) | (large) | Clouds.png | Cloud background (Sky Race) |
| Magnifyer.max | (large) | (large) | (6 textures) | Magnifying glass |
| Magnifyer-Cut.max | (large) | (large) | (6 textures) | Cut version |
| TrapDoor.max | (large) | (large) | (6 textures) | Trapdoor |
| TrapDoor-Cut.max | (large) | (large) | (6 textures) | Cut version |
| PopCylinder.max | (large) | (large) | (6 textures) | Pop cylinder |

#### Master Race Objects (Source Levels/Master/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| X-Spinner.max | (large) | (large) | (2 textures) | Spinner obstacle |
| X-Spinner-Cut.max | (large) | (large) | (2 textures) | Cut version |
| X-BreakBridge.max | (large) | (large) | (3 textures) | Breakable bridge |
| Gluebie.max | (large) | (large) | (13 textures) | Gluebie (Master version) |

#### Up Race Objects (Source Levels/Up/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Lifter.max | 8 | 6 | PurpleChecker.bmp | Lifter (very simple geometry) |
| Lifter-Cut.max | 65 | 63 | (4 textures) | Cut version with more detail |
| SpeedRoller.max | 30 | 17 | SpeedCylinder.png | Speed roller |

#### Wobbly Race Objects (Source Levels/Wobbly/)

| File | Verts | Faces | Textures | Notes |
|------|-------|-------|----------|-------|
| Wobbler.max | 872 | 779 | BrightGreenChecker.bmp | Wobbler platform |
| Wavy1.max | 116 | 84 | (5 textures) | Wavy platform |
| Loopy1.max | 344 | 312 | (5 textures) | Loop platform |
| Loopy1-Cut.max | 490 | 312 | (5 textures) | Cut version |

## All Unique Textures (108)

### Checker Textures
8Ball.bmp, 8Ball.png, BlackChecker.png, BlackCheckerFlag.png, BlueChecker.bmp, BlueCheckerFlag.png, BrightGreenChecker.bmp, BrightGreenCheckerFlag.png, GreenChecker.bmp, GreenCheckerFlag.png, GreyOutlineChecker.png, GlassCheckerFlag.png, GoldCheckerFlag.png, OrangeChecker.bmp, OrangeCheckerFlag.png, PinkChecker.bmp, PoleChecker.png, PurpleChecker.bmp, RedBlueChecker.bmp, RedChecker.bmp, RedChecker.png, RedCheckerFlag.png, SkyChecker.png, SkyCheckerFlag.png, ToobChecker.png, ToobCheckerFlag.png, WhiteCheckerFlag.png, YellowCheckerFlag.png, YelllowChecker.png

### Brick Textures
bluebrick.png, brightgreenbrick.png, brownbrick.png, greenbrick.png, orangebrick.png, purplebrick.png, redbrick.png, toobbrick.png

### Decal/Arrow Textures
Arrow1.png, Arrow2.png, Arrow3.png, Arrow-White.png, ArrowCurve1.png, ArrowCurve2.png, NeonArrow.png, OddArrow.png, OddArrow2.png, StumpyArrow.png, YellowArrow.png, ZigZagArrow.png, Decal-Start.png, Decal-Warning.png, Bullseye.png

### Entity Textures
Chrome.png, Clouds.png, Dizzies.png, FanSwirl.png, FunBall.png, GearSwirl.png, HamsterBall.png, Ice.png, Icicles.png, Lock.png, LockTile.png, MouseDude.png, GerbilDude.png, HamsterDude.png, Saw.png, saw.png, sawblade.png, sawface.png, SmashGlass.png, SpeedCylinder.png, Tar.png, TarBlot.png, TarSplotch.png, tarsplotch.png, TenBonus.png, FiveBonus.png

### Wood/Metal Textures
Burloak.jpg, gloswood.jpg, Lakerem.jpg, Lakerem2.jpg, LightWood.jpg, Oak1.tga, balse2.jpg, Wood.png, TrapBottom.jpg, TrapMetal.jpg, TrapSide.jpg, TrapSpring.jpg, TrapTop.jpg, texture01.jpg

### Character Textures
Hamster.jpg, Glare.png, PurpleEye.png, Dawggie.png, Bonk.png

### Other
Billiard-1.png, Felt.png, goal.png, goal-round.png, RedBlueChecker.bmp, redbluechecker.bmp, Sphere01CompleteMap.tga, Sphere01DiffuseMap.tga, YellowRing.png

## Development History Insights

### Computer Names Reveal Dev Timeline
- **RAPTISOFT** — John Raptis's main dev machine (most files)
- **MAINDEV** — Secondary dev machine
- **CORE2DEV** — Core 2 era machine (later files?)
- **GATEWAY** — Gateway computer (early files?)

### File Naming Patterns
- **"-Cut" suffix** — Simplified/cut versions of objects (lower poly or removed features)
- **"-Compiled" suffix** — Compiled/optimized versions
- **"-Optimized" / "-SuperOptimized"** — Progressively optimized meshes
- **"-Merged" / "-Shrunk"** — Merged and reduced geometry
- **"-Renderable"** — Final renderable versions
- **"Level3-" prefix** — Dizzy Race objects (Level3 = Dizzy in internal numbering)
- **"Level4-" prefix** — Tower Race objects (Level4 = Tower)
- **"OLD" prefix** — Older versions kept for reference
- **"(Ref)" suffix** — Reference versions

### Level Numbering (Internal)
Based on file names and folder structure:
1. Level1 = Warm-Up
2. Level2 = Intermediate
3. Level3 = Dizzy
4. Level4 = Tower
5. Level5 = Expert
6. Level6 = Master
7. Level7 = Wobbly
- "Cascade" = Beginner (earlier name)
- "Dark" = Neon
- "Spaghetti" = Odd
- "Unusual" = Up

## .MAX File Format Notes

.MAX files are **OLE2 Compound Documents** (same container format as .doc/.xls). Each file contains 8 internal streams:

1. `\x05DocumentSummaryInformation` — Metadata
2. `\x05SummaryInformation` — Summary metadata
3. `ClassData` — 3ds Max class data (binary, proprietary)
4. `ClassDirectory3` — Class directory
5. `Config` — Configuration
6. `DllDirectory` — Plugin DLL references
7. `Scene` — Main scene data (binary, proprietary — contains geometry, materials, objects)
8. `VideoPostQueue` — Video post-processing

### What Can Be Extracted Without 3ds Max
- ✅ Texture references (filenames)
- ✅ Vertex/face counts (from SummaryInformation)
- ✅ Object names
- ✅ Material names
- ✅ Bone names (for rigged models)
- ✅ 3ds Max version and computer name
- ❌ Actual vertex coordinates (stored in proprietary binary in Scene stream)
- ❌ UV coordinates
- ❌ Animation keyframes
- ❌ Full material properties (colors, shininess values)

### 3ds Max Plugins Referenced
All files reference these plugins:
- ViewportManager.gup
- CustAttribContainer.dlo
- mtl.dlt / mtlgen.dlt
- samplers.dlh
- shaders.dlb
- ctrl.dlc
- EPoly.dlo (Editable Poly)
- kernel.dlk
- rend.dlr

Some files also reference:
- bonesDef.DLM (Bone rigging)
- msmooth.dlm (Mesh smooth)
- mods.dlm (Modifiers)
- prim.dlo (Primitives)
- MultiRes.dlm (Multi-resolution)
- CurveControl.dlu (Curve control)

## Interesting Findings

1. **Mouse.max** references `Lakerem.jpg` (not `Hamm.png` as the compiled .MESH does). The .MAX source has a different texture than the game-ready version.

2. **Eye.max** contains 7 texture references including game-level textures (GreenChecker, Arrow2, etc.) — it seems to have been built within a level scene rather than as a standalone mesh.

3. **Cannon.max, Cannon-JustCannon.max, Cannon-JustBalloon.max** — The Cannon entity was modeled in two parts (cannon + balloon) that could be used separately or together.

4. **Level3-TwirlPipe.max** is the largest Dizzy file at 1.19 MB — the twirl pipe is the most complex single object.

5. **Dizzy-Renderable.max** and the "SuperOptimized" version are 9.3 MB each — the largest level files in the pack.

6. **BoxArt.max** (1.93 MB, 8980 verts) — likely used for promotional box art renderings.

7. **Title.max** (2.44 MB, 10 textures) — the title screen 3D scene.

8. **Secret.max** has only 8 verts and 6 faces — the secret bonus trigger is a tiny piece of geometry.

9. **GoldenWeasel.max** has 13,809 verts — by far the most complex mesh model. Likely a trophy or special reward model.

10. **Hamster-TitlePose.max** has 1812 verts vs 541 for the base model — the title screen pose has 3x the geometry detail for close-up rendering.

11. **Vaccuum.max** is misspelled (double 'c') — this typo carried through from the source files.

12. **"Lifter" appears in both Unusual/ and Up/ folders** — suggesting the Lifter object was shared between Up Race and an earlier "Unusual" version of the level.

---

*Documentation generated from analysis of 227 .MAX files using OLE2 container parsing. For full geometry extraction, 3ds Max or a specialized parser would be required.*
