# Timer/HUD System & Asset Manifest

## Architecture Overview

The game's resource loader (misnamed `TimerDisplay` at 0x42A8C0) creates a `LoadingScreenGadget` (0x3628 bytes) which serves as both:
1. Loading screen with progress indicator
2. Asset manager for the entire game

The timer/HUD system draws on top of the 3D scene during gameplay:
- Time pool display (with timerblot.png background)
- Race name
- Safe circle indicator (safecircle.png)
- Medal/rank icons (bronze/silver/gold/goldenweasel)

## Resource Loading Pipeline

### LoadingScreenGadget
Created once at startup (0x3628 bytes). Loads ALL game assets via vtable dispatch:

| Vtable Offset | Function | Parameter |
|---------------|----------|-----------|
| +0x48 | LoadTexture(destination, filename, mirror_flag) | .png/.bmp → Direct3D texture |
| +0x4C | LoadMesh(destination, filename) | .x mesh → D3DX mesh |
| +0x50 | LoadLevel(destination, filename) | Level → MeshWorld |
| +0x54 | AttachLevel(destination, source) | Attach secondary level to primary |
| +0x58 | LoadTextureSimple(destination, filename) | Texture without mirror variant |
| +0x5C | LoadFont(destination, fontname) | Font from fonts/ directory |
| +0x60 | LoadSound(destination, filename, channel_count) | Sound with channel pool count |

### Loading Flow
```
App_ResourceLoader(app):
  1. LoadingScreenGadget_Ctor(0x3628 bytes)
  2. app->loading_screen = gadget  // App+0x22C
  
  PHASE 1: Fonts
    - fonts\showcardgothic28 (title text, large)
    - fonts\arialnarrow12bold (small text)
    - fonts\showcardgothic14 (medium text)
    - fonts\showcardgothic72 (huge numbers / timer display)
    - fonts\showcardgothic16 (HUD text)
  
  PHASE 2: UI Textures (titletext, hammy, blueblot, bluecircle)
  PHASE 3: Race Textures (signs, goals, locktiles, arrows, checkers, bricks)
  PHASE 4: Ball Meshes (Sphere, SphereBreak, Hamster anims, 8Ball, FunBall, Bell, Dizzy)
  PHASE 5: Level Prefabs (MouseTrap, Secret, Secret-Unlock, Trapdoors, PopupSign)
  PHASE 6: Object Meshes (tarbubble, fanblades, sawblade, dawgshoe, mace)
  PHASE 7: HUD Textures (weaselbox, ballborder, timerblot, chrome, medals)
  PHASE 8: Tournament Textures (tourney-* for each race)
  PHASE 9: Result Textures (Burst, Lost, Winner, Winner2p, title1-4)
  PHASE 10: Sound Effects (55 sounds with channel counts)
  PHASE 11: Endgame Textures (settext, gotext)
  12. Menu_MergeAllLists()
  13. Scene_AddObject(app->scene, gadget)
```

## Complete Asset Manifest

### Fonts (5 fonts)
| App Offset | Font Name | Usage |
|------------|-----------|-------|
| +0x318 | showcardgothic28 | Title screens |
| +0x31C | showcardgothic14 | Medium dialogue |
| +0x320 | showcardgothic16 | HUD text |
| +0x324 | arialnarrow12bold | Small labels |
| +0x328 | showcardgothic72 | Timer display |

### Ball Meshes (14 meshes)
| App Offset | Mesh | Description |
|------------|------|-------------|
| +0x244 | Sphere | Standard ball |
| +0x248 | SphereBreak1 | Ball breaking animation 1 |
| +0x24C | SphereBreak2 | Ball breaking animation 2 |
| +0x250 | Hamster-Waiting | Hamster idle inside ball |
| +0x254 | Hamster-trot1 | Walk animation 1 |
| +0x258 | Hamster-trot2 | Walk animation 2 |
| +0x25C | Hamster-trot3 | Walk animation 3 |
| +0x260 | RBGlare | Light reflection on ball |
| +0x264 | Sphere+Tar | Ball with tar coating |
| +0x268 | 8Ball | 8-ball penalty ball |
| +0x26C | FunBall | Fun/bonus ball |
| +0x270 | Bell | Bell object |
| +0x274 | Dizzy | Dizzy animation |

### Race Textures (16 categories × 2-3 variants each)
Each race has a checker + brick texture pair:
- pink, blue, green, yellow, grey-outline, red, orange, bright-green
- toob (tube race), sky, purple, brown, black

Goal textures: goal.png, goal-lit.png, goal-mirrored.png, goal-lit-mirrored.png
Round goal variants: goal-round*.png

### Tournament Race Thumbnails (14 races)
| App Offset | Race Name |
|------------|----------|
| +0x3B4 | Beginner |
| +0x3B8 | Cascade |
| +0x3BC | Intermediate |
| +0x3C0 | Dizzy |
| +0x3C4 | Tower |
| +0x3C8 | Up |
| +0x3CC | Neon |
| +0x3D0 | Expert |
| +0x3D4 | Odd |
| +0x3D8 | Toob |
| +0x3DC | Wobbly |
| +0x3E0 | Glass |
| +0x3E4 | Sky |
| +0x3E8 | Master |
| +0x3EC | Impossible |

### Sound Effects (55 sounds)
| App Offset | Sound | Channels |
|------------|-------|----------|
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

## Timer System

### RumbleBoard Timer (Ball+0x264)
Each ball has a `RumbleBoard` timer component (16 bytes):
- Used in rumble/arena mode for round timing
- `RumbleBoard_InitTimer` resets the timer
- Timer counts down during gameplay
- When timer reaches zero: round ends

### Tournament Time Pool
- `PlayerProfile+0x5E4` = total accumulated time (float, for ranking)
- `App+0x5E8` = active time pool (float)
- `App+0x5F4` = (unknown timing float)
- Time pool decreases during race; when it hits zero = "LOST TOURNAMENT"
- Warm-up race (race 1) does NOT affect time pool

### Timer Display
- Font: showcardgothic72 (large digits)
- Background: timerblot.png
- Positioned in HUD overlay layer (alpha pass)
- Displayed as minutes:seconds remaining

## HUD Elements

| Element | Texture | Location |
|---------|---------|----------|
| Timer background | timerblot.png | Top-center |
| Safe circle | safecircle.png | Progress indicator |
| Ball border | ballborder.png | Ball highlight ring |
| Ball burner | ballburner.png | Speed effect overlay |
| Sweat drop | sweat.png | Ball strain indicator |
| Star burst | star.png | Achievement effect |
| Dust cloud | dust.png | Impact effect |
| Chrome ball | chrome.png / chromeshadow.png | Premium ball skin |
| Medal (bronze) | bronze-small.png, bronze-icon.png | Tournament score |
| Medal (silver) | silver-small.png, silver-icon.png | |
| Medal (gold) | gold-small.png, gold-icon.png | |
| Golden weasel | goldenweasel.png, goldenweasel-icon.png | Best medal |
| Lock icon | lock.png | Locked content |
| Rank badge | textures\ranks\%d.jpg | Post-race rank display |

## Key Address Map

| Address | Function | Description |
|---------|----------|-------------|
| 0x42A8C0 | App_ResourceLoader | Load ALL game assets |
| 0x458E60 | RumbleBoard_InitTimer | Reset round timer |

## Reimplementation Notes (SDL2)

### Asset Loading
- Use a manifest file (JSON/YAML) instead of hardcoded loading
- SDL2: `IMG_Load()` for textures, `TTF_OpenFont()` for fonts
- For meshes: custom mesh format or GLTF2
- For sounds: `Mix_LoadWAV()` / `Mix_Music`

### Timer System
```cpp
class TimerSystem {
    float time_pool;        // Tournament time remaining
    float race_elapsed;     // Current race time
    bool is_warmup;         // Race 1 doesn't affect pool
    
    void Update(float dt) {
        race_elapsed += dt;
        if (!is_warmup) time_pool -= dt;
        if (time_pool <= 0) OnTournamentLost();
    }
    
    void Render() {
        // Render timer display with showcardgothic72 font
        // Background with timerblot.png
        // Format as MM:SS
    }
};
```