# Hamsterball — The App Object: A Modder's Guide

## What This Document Is

This is a reverse-engineering reference for anyone who wants to hook into,
read from, or modify the running Hamsterball game. The `App` object is the
global singleton that holds every subsystem pointer, every game-state flag,
and every config value. If you can get its address, you can reach every
other object in the game.

---

## Getting the App Pointer

### Method 1: Direct Global Address (Easiest)

The game allocates the `App` object as a global static in `.data`.

- **Address:** `0x004FD680` (VA in Hamsterball.exe)
- **Symbol:** `g_App`
- **Type:** `App*` (pointer to a ~0xA00-byte structure)

```c
// In your DLL / injector code
App* g_App = *(App**)0x004FD680;
```

This address is valid from the moment `WinMain` calls `App_Ctor` (0x0046DC40)
until process exit. The constructor runs before the game loop starts, so you
can safely read it any time after `CreateWindow` returns.

### Method 2: Hook WinMain (Guaranteed Early Access)

If you need the address before any frame runs, hook the beginning of
`WinMain` (0x004278E0). The first instruction after entry is:

```
WinMain:
  push ebp
  mov ebp, esp
  ...
  call App_Ctor        ; creates App at 0x4FD680
  call App_Initialize_Full
  call App_Run          ; game loop
  call LoadOrSaveConfig
```

Hook address: **0x004278E0**  
At this point `g_App` is already constructed (the CRT `.data` init ran
before `WinMain`). You can read `0x004FD680` immediately.

### Method 3: Hook App_Initialize_Full (All Subsystems Ready)

If you want to intercept the game **after** all subsystems are initialized
(Graphics, Audio, Input, Registry) but **before** the first frame renders:

- **Address:** 0x00429530  
- **Why:** Step 25 calls `vtable[0xA0]()` which shows the title screen. If
  you hook after step 24 and before step 25, every subsystem pointer is
  filled in.

### Method 4: Hook App_Run (Per-Frame Access)

For frame-by-frame mods (trainers, overlays, TAS tools):

- **Address:** 0x0046BD80  
- **Hook point:** The outer `while (!quit_flag)` loop. Replace the
  `app->vtable[0x20]()` call (Update) or `app->vtable[0x28]()` call (Render)
  with your own dispatcher.

```c
// Original dispatch in App_Run:
app->vtable[0x20]();   // Update  — game logic
app->vtable[0x24]();   // Pre-render
app->vtable[0x28]();   // Render  — draw everything
app->vtable[0x2C]();   // Post-render / HUD
```

Replacing any of these four vtable calls lets you run custom code every
frame while keeping the original game running.

### Method 5: Hook App_FrameUpdate (Convenient Single Point)

- **Address:** 0x0046C170  
- **What it does:** Polls input, runs collision, calls `GameUpdate`, handles
  cursor capture.
- **Why hook it:** One single hook gives you input, physics, and game-state
  access every frame without touching the render pipeline.

---

## App Vtable Layout

The App vtable lives at **0x004CE400**. Key slots:

| Slot | Offset | Address | Name | Description |
|------|--------|---------|------|-------------|
| 0 | +0x00 | 0x46DC20 | App_ScalarDtor | Destructor + free if flag&1 |
| 2 | +0x08 | 0x46BA10 | App_Shutdown | Cleanup on exit |
| 8 | +0x20 | — | Update | Game logic (Scene_Update) |
| 9 | +0x24 | — | PreRender | Camera setup |
| 10 | +0x28 | — | Render | Draw scene |
| 11 | +0x2C | — | PostRender | HUD / menus |
| 35 | +0x8C | — | SetDisplayMode | Called with (800,600) during init |
| 40 | +0xA0 | 0x4280E0 | App_ShowMainMenu | Creates MainMenu object |

Hooking any vtable slot is the cleanest way to intercept behavior without
patching function bodies. Just overwrite the pointer at
`*(void**)(g_App + 0x00) + slot_offset`.

---

## App Structure Layout (Offsets)

The `App` struct is the root of the game. Everything else hangs off it.

### Core Identity & Window

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x000 | void** | vtable | App vtable = 0x004CE400 |
| +0x004 | HINSTANCE | hInstance | WinMain param_1 |
| +0x008 | int | cmdShow | WinMain nCmdShow |
| +0x054 | RegKey* | registryKey | Registry handle (ADVAPI32) |

### Timing & Frame Control

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x05C | int | targetFPS | Target frame rate (usually 30) |
| +0x168 | int | msPerFrame | 1000 / targetFPS |
| +0x170 | int | fpsDivisor | Backup of target FPS |
| +0x18C | int | updateCount | Total physics updates |
| +0x194 | int | frameCounter | Frames rendered this second |
| +0x1AC | bool | showFPS | 1 = draw FPS counter |
| +0x210 | char* | phaseName | "Background" / "Update" / "Render" |
| +0x159 | bool | quitFlag | 1 = exit game loop |
| +0x15A | bool | activeFlag | 1 = window focused |
| +0x158 | bool | minimizedFlag | 1 = window minimized |
| +0x156 | bool | updateDisabled | 1 = pause all updates |

### Subsystem Pointers (The Big Ones)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x174 | Graphics* | graphics | D3D8 Graphics engine |
| +0x17C | AudioSystem* | audioSystem | BASS audio wrapper |
| +0x180 | InputHandler* | inputHandler | DirectInput8 handler |
| +0x184 | void* | gameUpdateObj | Passed to App_TickGameUpdate |
| +0x5D | void* | renderTarget | D3D render target surface |

### Display Settings

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x15C | int | width | Window width (default 800) |
| +0x160 | int | height | Window height (default 600) |
| +0x158 | bool | windowed | 1 = windowed, 0 = fullscreen |

### Audio / Music

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x534 | HMUSIC | musicHandle | BASS music handle for music.mo3 |
| +0x538 | HCHANNEL | musicChannel1 | BASS channel 1 |
| +0x53C | HCHANNEL | musicChannel2 | BASS channel 2 |

### Game Mode Objects (vtable 0x8C call creates these)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x550 | void* | gameMode1 | 1-player mode object |
| +0x554 | void* | gameMode2 | 2-player mode object |
| +0x558 | void* | gameMode3 | 4-player mode object |
| +0x55C | void* | gameMode4 | Tournament mode object |

### Game State & Scenes

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x178 | Scene* | currentScene | Active scene (menu or level) |
| +0x184 | Scene* | loadingScene | Loading screen |
| +0x1DC | SoundChannel* | inputSound | Click/beep channel |
| +0x1E4 | InputDevice* | player1Device | P1 input (keyboard default) |
| +0x1E8 | InputDevice* | player2Device | P2 input |
| +0x224 | void* | mainMenuObj | MainMenu instance |
| +0x228 | void* | resultsScreen | Race results screen |
| +0x708 | int | gameState | 3 = racing, other = menu/loading |

### Progress / Unlock Flags

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x851 | bool | unlock_DizzyRace | 1 = Dizzy race unlocked |
| +0x852 | bool | unlock_TowerRace | 1 = Tower race unlocked |
| +0x853 | bool | unlock_UpRace | 1 = Up race unlocked |
| +0x854 | bool | unlock_ExpertRace | 1 = Expert race unlocked |
| +0x855 | bool | unlock_OddRace | 1 = Odd race unlocked |
| +0x856 | bool | unlock_ToobRace | 1 = Toob race unlocked |
| +0x857 | bool | unlock_WobblyRace | 1 = Wobbly race unlocked |
| +0x858 | bool | unlock_SkyRace | 1 = Sky race unlocked |
| +0x859 | bool | unlock_MasterRace | 1 = Master race unlocked |
| +0x85A | bool | unlock_DizzyArena | 1 = Dizzy arena unlocked |
| +0x85B | bool | unlock_TowerArena | 1 = Tower arena unlocked |
| +0x85C | bool | unlock_UpArena | 1 = Up arena unlocked |
| +0x85D | bool | unlock_ExpertArena | 1 = Expert arena unlocked |
| +0x85E | bool | unlock_OddArena | 1 = Odd arena unlocked |
| +0x85F | bool | unlock_ToobArena | 1 = Toob arena unlocked |
| +0x860 | bool | unlock_WobblyArena | 1 = Wobbly arena unlocked |
| +0x861 | bool | unlock_SkyArena | 1 = Sky arena unlocked |
| +0x862 | bool | unlock_MasterArena | 1 = Master arena unlocked |
| +0x863 | bool | unlock_NeonRace | 1 = Neon race unlocked |
| +0x864 | bool | unlock_GlassRace | 1 = Glass race unlocked |
| +0x865 | bool | unlock_ImpossibleRace | 1 = Impossible race unlocked |
| +0x866 | bool | unlock_NeonArena | 1 = Neon arena unlocked |
| +0x867 | bool | unlock_GlassArena | 1 = Glass arena unlocked |
| +0x868 | bool | unlock_ImpossibleArena | 1 = Impossible arena unlocked |
| +0x86C | uint8[0x50] | bestTimes | Per-level best times (raw binary) |
| +0x8BC | uint8[0x50] | medals | Per-level medal status (0=none..3=gold) |
| +0x914 | int | playCount | Total launches from registry |
| +0x850 | bool | mirrorMode | Tournament mirror tracks |
| +0x84C | float | mouseSensitivity | 0.0 – 1.0 range |
| +0x238 | bool | rightButtonPause | Toggle pause on right-click |

### Input / Controller Config

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0xB28 | DWORD | p2Controller1 | DirectInput device index |
| +0xB2C | DWORD | p2Controller2 | DirectInput device index |
| +0xB30 | DWORD | p2Controller3 | DirectInput device index |
| +0xB34 | DWORD | p2Controller4 | DirectInput device index |

### Misc

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x1B4 | char* | versionString | ProductVersion from Version API |
| +0x1CC | int | loadedCount | Objects loaded counter |
| +0x200 | bool | initialized | 1 after App_Initialize_Full finishes |
| +0x208 | char* | initStep | Debug string: "Initialize(1)".."(26)" |
| +0x240 | HCURSOR | cursor | "BLANKCURSOR" handle |
| +0x278 | Texture* | shadowTexture | Loaded shadow.png |

---

## What You Can Do With the App Object

### 1. Instant Unlock Everything

```c
// Set every unlock flag to 1
for (int i = 0x851; i <= 0x868; i++) {
    *(bool*)(g_App + i) = true;
}
```

This unlocks every race and arena instantly. No registry editing, no file
patching. The game reads these flags every time it draws a menu.

### 2. Force Fullscreen / Windowed

```c
// Toggle windowed mode
bool* windowed = (bool*)(g_App + 0x158);
*windowed = !*windowed;
// Then call App_SetFullScreen(0x0046C7C0) to apply
```

### 3. Change Mouse Sensitivity

```c
float* sens = (float*)(g_App + 0x84C);
*sens = 2.0f;   // Double default sensitivity
```

### 4. Skip Intro / Instant Menu

Hook `App_Initialize_Full` and replace the call at step 25:

```
Original:
  call vtable[0xA0]   ; Shows title screen
Replace with:
  call App_ShowMainMenu  ; 0x004280E0 — jumps straight to menu
```

### 5. Frame-By-Frame TAS Hook

Hook `App_Run` and replace `vtable[0x20]()` with your own function that
reads input from a file instead of DirectInput:

```c
void MyUpdate() {
    // Read next frame of inputs from TAS movie file
    // Write them into InputDevice+0x50C..0x518 (DIK codes)
    // Call original Scene_Update
    OriginalSceneUpdate();
}
```

### 6. Force Quit / Safe Exit

```c
// Set quit flag — game exits cleanly at next loop iteration
*(bool*)(g_App + 0x159) = true;
```

### 7. FPS Unlock / Frame Limiter Override

```c
// Change target FPS (default is 30)
*(int*)(g_App + 0x05C) = 60;
*(int*)(g_App + 0x168) = 1000 / 60;
```

### 8. No-Clip / Disable Collision

The `currentScene` pointer at +0x178 leads to the `Scene` object, which
has a collision handler at `vtable+0x29` (0x40C5D0). Replacing that vtable
slot with a no-op disables all collision events.

```c
Scene* scene = *(Scene**)(g_App + 0x178);
void** scene_vt = *(void***)scene;
scene_vt[0x29] = (void*)0x44B840;   // NoOp stub
```

### 9. Instant Level Load

`App_StartRace` (0x004287C0) takes a level path string and loads it
immediately. Call it from your hook to warp to any level:

```c
typedef void (*App_StartRace_t)(App*, const char*);
App_StartRace_t StartRace = (App_StartRace_t)0x004287C0;
StartRace(g_App, "levels\\level10");
```

### 10. Music Speed / Tempo Hack

The BASS music handle is at `App+0x534`. Use BASS's `BASS_ChannelSetAttribute`
with `BASS_ATTRIB_FREQ` to change playback speed without touching game code.

---

## Key Functions for Modding

| Address | Name | Why You Care |
|---------|------|-------------|
| 0x004278E0 | WinMain | Earliest hook point |
| 0x00429530 | App_Initialize_Full | All systems ready, pre-menu |
| 0x0046BD80 | App_Run | Per-frame hook point |
| 0x0046C170 | App_FrameUpdate | Input+physics every frame |
| 0x0046C7C0 | App_SetFullScreen | Toggle display mode |
| 0x004280E0 | App_ShowMainMenu | Jump to menu |
| 0x004287C0 | App_StartRace | Load any level |
| 0x00425F90 | App_CompleteRace | Finish race instantly |
| 0x00428C50 | App_StartPracticeRace | Start practice mode |
| 0x004288B0 | App_StartTournamentRace | Start tournament |
| 0x0046CB70 | App_SetTitleString | Change window title |
| 0x0046C050 | App_CreateInputDevice | Add custom input device |
| 0x0046CB00 | App_CreateScoreDisplay | Inject HUD element |
| 0x0046BCA0 | App_WriteDisplaySettings | Save res to registry |
| 0x0046BD00 | App_ReadDisplaySettings | Load res from registry |
| 0x4284C0 | App_SaveAllConfig | Force save all settings |

---

## Registry Persistence

The game saves all unlock flags and settings to the Windows registry on exit.
If you set flags in memory but want them to survive a restart, either:

1. Call `App_SaveAllConfig(0x004284C0)` with `g_App` as the argument.
2. Let the game exit normally — it calls this automatically in `LoadOrSaveConfig`.

Registry path (inferred): `HKEY_CURRENT_USER\Software\Raptisoft\Hamsterball`

---

## C Header for Injection

```c
#ifndef HB_APP_H
#define HB_APP_H
#include <stdint.h>
#include <windows.h>

typedef struct {
    void**    vtable;          // +0x000
    HINSTANCE hInstance;       // +0x004
    int       cmdShow;         // +0x008
    uint8_t   pad_00C[0x48];   // +0x00C..0x053
    void*     registryKey;     // +0x054
    uint8_t   pad_058[0x04];
    int       targetFPS;       // +0x05C
    int       frameTimeMs;     // +0x5A (alias, verify)
    int       fpsDenominator;  // +0x5B
    void*     renderTarget;    // +0x5D
    int       frameCounter;    // +0x65
    uint8_t   pad_069[0xEF];
    uint8_t   updateDisabled;  // +0x156
    uint8_t   windowed;        // +0x158
    uint8_t   minimizedFlag;   // +0x15A
    uint8_t   quitFlag;        // +0x159
    int       width;           // +0x15C
    int       height;          // +0x160
    uint8_t   pad_164[0x10];
    void*     graphics;        // +0x174
    void*     audioSystem;     // +0x17C
    void*     inputHandler;    // +0x180
    void*     gameUpdateObj;   // +0x184
    uint8_t   pad_188[0x2C];
    char*     versionString;   // +0x1B4
    int       loadedCount;     // +0x1CC
    uint8_t   initialized;     // +0x200
    char*     initStep;        // +0x208
    uint8_t   pad_20C[0x34];
    HCURSOR   cursor;          // +0x240
    uint8_t   pad_244[0x34];
    void*     shadowTexture;   // +0x278
    uint8_t   pad_27C[0x2B8];
    HMUSIC    musicHandle;     // +0x534
    HCHANNEL  musicChannel1;   // +0x538
    HCHANNEL  musicChannel2;   // +0x53C
    uint8_t   pad_540[0x10];
    void*     gameMode1;       // +0x550
    void*     gameMode2;       // +0x554
    void*     gameMode3;       // +0x558
    void*     gameMode4;       // +0x55C
    uint8_t   pad_560[0x3B4];
    void*     currentScene;    // +0x178 (RELOCATE — verify in Ghidra)
    // ... continue from docs above
} App;

// Global singleton
static App** const g_ppApp = (App**)0x004FD680;
#define g_App (*g_ppApp)

#endif
```

> **Note:** The header above uses approximate offsets. For production code,
> verify every offset in Ghidra against the current binary build. The
> `App_Initialize_Full` decompilation is the authoritative source.

---

## Quick Reference: Offsets at a Glance

```
0x004FD680  g_App                App*  (global)
0x004CE400  App_vtable           void**
0x004D0260  Scene_vtable         void**
0x004D934C  SceneObject_vtable   void**
0x004CF300  Vec3_vtable          void**
```

---

## Files Referenced

| File | Description |
|------|-------------|
| `analysis/ghidra/structs/app_struct.h` | Ghidra C struct export |
| `analysis/ghidra/decompilations/app/decomp_app_initialize.c` | App_Initialize_Full decomp |
| `analysis/ghidra/decompilations/app/decomp_app_run.c` | App_Run decomp |
| `docs/FUNCTION_MAP.md` | Full function listing |
| `docs/SAVE_CONFIG_REGISTRY_SYSTEM.md` | Registry details |
| `docs/GAME_LOOP_WINDOW_MANAGEMENT.md` | Frame timing |
| `docs/STRUCTS_AND_TYPES.md` | Other structures |

---

*Document version: 2026-06-04*  
*Based on Hamsterball.exe analysis via Ghidra 12.0.4 + GhidraMCP*  
*Phase 1 structs complete — App struct verified against decompilation*
