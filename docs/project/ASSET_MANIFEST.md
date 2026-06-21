# Asset Loading & Resource Manifest

## Overview
TimerDisplay (0x4298C0) is actually **App_ResourceLoader** — it's the master asset loading
function that loads ALL game assets during the loading screen phase. It creates a
LoadingScreenGadget (0x3628 bytes) and calls its vtable methods to load resources.

## LoadingScreenGadget Vtable Methods

| Offset | Method | Purpose |
|--------|--------|---------|
| +0x48 | LoadMeshWithCollision | Load mesh + collision data (2nd arg=collision flag) |
| +0x4C | LoadMesh | Load mesh without collision |
| +0x50 | LoadLevel | Load MESHWORLD level |
| +0x54 | LoadLevelLinked | Load level + link to another level |
| +0x58 | LoadTexture | Load texture (PNG/BMP) |
| +0x5C | LoadFont | Load bitmap font |
| +0x60 | LoadSound | Load OGG sound effect (2nd arg=channel count) |

## Complete Asset Manifest

### Fonts (5)
| App Offset | Path | Purpose |
|-----------|------|---------|
| +0x318 | fonts\showcardgothic28 | Main title/heads-up font (28pt) |
| +0x324 | fonts\arialnarrow12bold | UI body text (12pt bold) |
| +0x31C | fonts\showcardgothic14 | Small labels (14pt) |
| +0x328 | fonts\showcardgothic72 | Large display (72pt) |
| +0x320 | fonts\showcardgothic16 | Medium labels (16pt) |

### Title Textures (3)
| App Offset | Path | Purpose |
|-----------|------|---------|
| +0x348 | titletext-left.png | Title screen left half |
| +0x34C | titletext-right.png | Title screen right half |
| +0x330 | textures\hammy1.png | Hamster character 1 |
| +0x334 | textures\hammy2.png | Hamster character 2 |
| +0x338 | textures\hammy3.png | Hamster character 3 |

### Sign & Goal Textures (18)
| App Offset | Path | Flag |
|-----------|------|------|
| +0x27C | sign-bewarethetar.png | 0 (no collision) |
| +0x280 | sign-bewarethetar.png | 1 (collision) |
| +0x284 | sign-bewarethetar-mirrored.png | 0 |
| +0x288 | arrow1.png | 0 |
| +0x28C | arrow1.png | 1 |
| +0x290 | arrow1-mirrored.png | 0 |
| +0x294 | goal.png | 0 |
| +0x298 | goal.png | 1 |
| +0x29C | goal-lit.png | 1 |
| +0x2A0 | goal-mirrored.png | 1 |
| +0x2A4 | goal-lit-mirrored.png | 1 |
| +0x2A8 | locktile.png | 0 |
| +0x2AC | locktile.png | 1 |
| +0x2B0 | locktile2.png | 1 |
| +0x2B4 | goal-round.png | 0 |
| +0x2B8 | goal-round.png | 1 |
| +0x2BC | goal-round-lit.png | 1 |
| +0x2C0 | goal-round-mirrored.png | 1 |
| +0x2C4 | goal-round-lit-mirrored.png | 1 |

### Checker/Brick Textures (18)
| Path | Purpose |
|------|---------|
| pinkchecker.bmp | Pink checker tile |
| bluechecker.bmp | Blue checker tile |
| bluebrick.png | Blue brick tile |
| greenchecker.bmp | Green checker tile |
| greenbrick.png | Green brick tile |
| yelllowchecker.png | Yellow checker tile (typo in original) |
| greyoutlinechecker.png | Grey outlined checker |
| redchecker.bmp | Red checker tile |
| redbrick.png | Red brick tile |
| orangechecker.bmp | Orange checker tile |
| orangebrick.png | Orange brick tile |
| brightgreenchecker.bmp | Bright green checker |
| brightgreenbrick.png | Bright green brick |
| toobchecker.png | Tube race checker |
| toobbrick.png | Tube race brick |
| skychecker.png | Sky race checker |
| purplechecker.bmp | Purple checker tile |
| purplebrick.png | Purple brick tile |
| brownbrick.png | Brown brick tile |
| blackchecker.png | Black checker tile |

### Ball/Mesh Textures (12)
| Path | Purpose |
|------|---------|
| blueblot.png | Blue blob background |
| blueblot2.png | Blue blob variant |
| bluecircle.png | Blue circle |
| chrome.png | Chrome ball texture |
| chromeshadow.png | Chrome ball shadow |
| weaselbox.png | Weasel ranking box |
| settext.png | Settings text |
| gotext.png | GO! text overlay |
| ballborder.png | Ball border frame |
| ballburner.png | Ball burner effect |
| sweat.png | Sweat drop sprite |
| raptisoftlogo.png | Developer logo |

### Medal/Icon Textures (8)
| Path | Purpose |
|------|---------|
| bronze-small.png | Bronze medal small |
| silver-small.png | Silver medal small |
| gold-small.png | Gold medal small |
| goldenweasel.png | Golden weasel award |
| bronze-icon.png | Bronze medal icon |
| silver-icon.png | Silver medal icon |
| gold-icon.png | Gold medal icon |
| goldenweasel-icon.png | Golden weasel icon |

### Tournament Thumbnails (15)
| Path | Level |
|------|-------|
| tourney-beginner.png | Beginner race |
| tourney-cascade.png | Cascade race |
| tourney-intermediate.png | Intermediate race |
| tourney-dizzy.png | Dizzy race |
| tourney-Tower.png | Tower race |
| tourney-Up.png | Up race |
| tourney-Neon.png | Neon race |
| tourney-Expert.png | Expert race |
| tourney-Odd.png | Odd race |
| tourney-Toob.png | Toob race |
| tourney-Wobbly.png | Wobbly race |
| tourney-Glass.png | Glass race |
| tourney-Sky.png | Sky race |
| tourney-Master.png | Master race |
| tourney-Impossible.png | Impossible race |

### Menu/Overlay Textures (8)
| Path | Purpose |
|------|---------|
| tournament.png | Tournament mode splash |
| timetrials.png | Time trial splash |
| partyrace.png | Party race splash |
| rodentrumble.png | Rodent Rumble splash |
| mirror.png | Mirror mode icon |
| Burst.png | Burst effect |
| Lost.png | "You Lost" overlay |
| Winner.png | "You Won" overlay |
| Winner2p.png | "Player Won" 2P overlay |
| title1-4.png | Title screen frames |
| unlock.png | Unlock notification |
| scoreball.png | Score ball sprite |

### Meshes (14)
| App Offset | Path | Purpose |
|-----------|------|---------|
| +0x244 | Meshes\Sphere | Main ball mesh |
| +0x248 | Meshes\SphereBreak1 | Ball break animation 1 |
| +0x24C | Meshes\SphereBreak2 | Ball break animation 2 |
| +0x260 | Meshes\RBGlare | Reflection glare mesh |
| +0x264 | Meshes\Sphere+Tar | Tar-stuck ball mesh |
| +0x250 | Meshes\Hamster-Waiting | Hamster idle pose |
| +0x254 | Meshes\Hamster-trot1 | Hamster walk frame 1 |
| +0x258 | Meshes\Hamster-trot2 | Hamster walk frame 2 |
| +0x25C | Meshes\Hamster-trot3 | Hamster walk frame 3 |
| +0x268 | Meshes\8Ball | 8-ball mesh (tournament) |
| +0x26C | Meshes\FunBall | Fun ball mesh (arena) |
| +0x270 | Meshes\Bell | Bell mesh (tournament) |
| +0x274 | Meshes\Dizzy | Dizzy powerup mesh |
| +0x584 | Meshes\GlassBonus | Glass bonus mesh |
| +0x588 | Meshes\GlassBonus-Smashed | Glass bonus smashed |
| +0x5A4 | Meshes\tarbubble | Tar bubble mesh |
| +0x5A8 | Meshes\fanblades | Fan blades mesh |
| +0x5AC | Meshes\fanbody | Fan body mesh |
| +0x5B0 | Meshes\sawblade | Saw blade mesh |
| +0x5B4 | Meshes\sawface | Saw face mesh |
| +0x5B8 | Meshes\sawface2 | Saw face variant |
| +0x5BC | Meshes\dawgshoe | Blockdawg shoe 1 |
| +0x5C0 | Meshes\dawgshoe2 | Blockdawg shoe 2 |
| +0x5C4 | Meshes\dawgshadow | Blockdawg shadow |
| +0x578 | Meshes\mousetrapshadow | Mouse trap shadow |

### Levels (7)
| App Offset | Path | Purpose |
|-----------|------|---------|
| +0x570 | Levels\MouseTrap | Mouse trap level |
| +0x57C | Levels\Secret | Secret/unlock level |
| +0x580 | Levels\Secret-Unlock | Secret unlock level |
| +0x594 | Levels\Level4-Trapdoor1 | Trapdoor level 1 |
| +0x598 | Levels\Level4-Trapdoor2 | Trapdoor level 2 |
| +0x58C | Levels\PopupSign | Popup sign level |
| +0x5C8 | Levels\Level6-Lifter | Lifter level |
| +0x584 | (linked to MouseTrap) | Mouse trap collision |
| +0x59C | (linked to Trapdoor1) | Trapdoor 1 collision |
| +0x5A0 | (linked to Trapdoor2) | Trapdoor 2 collision |
| +0x590 | (linked to PopupSign) | Popup sign collision |

### Sound Effects (55)
| App Offset | Path | Channels |
|-----------|------|----------|
| +0x43C | sounds\collide | 10 |
| +0x440 | sounds\roll | 10 |
| +0x444 | sounds\whistle | 1 |
| +0x448 | sounds\bumper | 10 |
| +0x44C | sounds\ballbreak | 5 |
| +0x450 | sounds\ballbreaksmall | 5 |
| +0x454 | sounds\thwomp | 2 |
| +0x458 | sounds\snap | 2 |
| +0x45C | sounds\popup | 2 |
| +0x460 | sounds\dropin | 2 |
| +0x464 | sounds\dropinshort | 2 |
| +0x468 | sounds\popout | 2 |
| +0x46C | sounds\pipebump1 | 10 |
| +0x470 | sounds\pipebump2 | 10 |
| +0x474 | sounds\pipebump3 | 10 |
| +0x478 | sounds\gearclank | 20 |
| +0x47C | sounds\bridgeslam | 2 |
| +0x480 | sounds\platformtick | 5 |
| +0x484 | sounds\gluestuck | 5 |
| +0x488 | sounds\bubble1 | 5 |
| +0x48C | sounds\bubble2 | 5 |
| +0x490 | sounds\wheelcreak | 2 |
| +0x494 | sounds\catapult | 2 |
| +0x498 | sounds\trapdoor | 2 |
| +0x49C | sounds\fwing | 2 |
| +0x4A0 | sounds\clink | 3 |
| +0x4A4 | sounds\whoosh | 3 |
| +0x4A8 | sounds\chomp | 1 |
| +0x4AC | sounds\fan-start | 10 |
| +0x4B0 | sounds\fan-blow | 10 |
| +0x4B4 | sounds\crack | 2 |
| +0x4B8 | sounds\crumble | 2 |
| +0x4BC | sounds\sawstartup | 2 |
| +0x4C0 | sounds\sawcut | 2 |
| +0x4C4 | sounds\minipop | 5 |
| +0x4C8 | sounds\bell | 3 |
| +0x4CC | sounds\zip | 2 |
| +0x4D0 | sounds\ting | 20 |
| +0x4D4 | sounds\shrink | 3 |
| +0x4D8 | sounds\grow | 3 |
| +0x4DC | sounds\tweet | 3 |
| +0x4E0 | sounds\creakyplatform | 20 |
| +0x4E4 | sounds\wubba | 5 |
| +0x4E8 | sounds\saw | 2 |
| +0x4EC | sounds\sawspeedy | 2 |
| +0x4F0 | sounds\dawgstep1 | 10 |
| +0x4F4 | sounds\dawgstep2 | 10 |
| +0x4F8 | sounds\dawgsmash | 10 |
| +0x4FC | sounds\sizzle | 2 |
| +0x500 | sounds\explode | 3 |
| +0x504 | sounds\vac-o-sux | 3 |
| +0x508 | sounds\speedcylinder | 2 |
| +0x50C | sounds\bonuspop | 5 |
| +0x510 | sounds\buzzbonus | 1 |
| +0x514 | sounds\breakbridge | 1 |
| +0x518 | sounds\unlock | 1 |
| +0x51C | sounds\NeonRide | 1 |
| +0x520 | sounds\NeonFlicker | 50 |
| +0x524 | sounds\ZoopDown | 2 |
| +0x528 | sounds\LightsOff | 2 |
| +0x52C | sounds\GlassBonus | 2 |

### RaceGoalReached Ctor (0x44C880)
When a player finishes a race:
- Sets title to "GOAL REACHED!"
- Loads "textures\ranks\weasel.png" rank sprite
- Updates best time in App+0x86C[player_index*4]
- 800ms timer (frames) for goal animation
- Medal thresholds hardcoded as integer values