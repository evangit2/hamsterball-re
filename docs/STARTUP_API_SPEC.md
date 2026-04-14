# Startup API Replication Spec

This document maps the original Hamsterball.exe startup sequence to our
open-source reimplementation. Each step is verified against Ghidra
decompilation of `App_Initialize_Full` (0x429530).

## Original Flow: WinMain (0x4278E0)

```
WinMain → App_Initialize_Full → App_Run → App_Shutdown
```

## App_Initialize_Full Steps (0x429530)

| Step | Original | Our Equivalent | Status |
|------|----------|----------------|--------|
| 1 | App_Initialize (0x46BB40) — 12 sub-steps: Registry, Window, DirectInput8, BASS audio, Graphics | SDL_Init, window, GL context, audio, input | ✅ |
| 2 | Set windowed mode flag (App+0x174->0x7d1=1) | SDL_WINDOW_RESIZABLE | ✅ |
| 3 | LoadCursorA(hInst, "BLANKCURSOR") | SDL_ShowCursor(0) during gameplay | ✅ |
| 4 | SetDisplayMode(800, 600) via vtable[0x8c] | SDL_CreateWindow 800x600 | ✅ |
| 5 | D3D device check + SetRenderState(D3DRS_LIGHTING, TRUE, SHADEMODE=3) | glEnable(GL_LIGHTING), glShadeModel(GL_SMOOTH) | ✅ |
| 6 | Graphics_FindOrCreateTexture("shadow.png") | texture_load("shadow") | ✅ |
| 7 | MusicChannel_LoadAndAppend("music\music.mo3") | Mix_LoadMUS (placeholder) | ⏳ |
| 8 | LoadJukebox("jukebox.xml") | TODO: parse jukebox.xml | ⏳ |
| 9-10 | RegKeyList_CopyFromSibling (music config channels) | N/A for open-source | ⬜ |
| 11 | Continue music setup | N/A | ⬜ |
| 12 | RegKey_Open | config_load() | ⏳ |
| 13 | RegKey_ReadDword("PlayCount") default=20 | config_defaults() | ⏳ |
| 14 | Set initialized flag (App+0x200=1) | g_state = GAME_STATE_MENU | ✅ |
| 15-16 | InputDevice(0, type=1) → keyboard | SDL keyboard state | ✅ |
| 17-18 | InputDevice(1, type=2) → mouse | SDL mouse events | ✅ |
| 19-20 | InputDevice(2, type=4) → joystick1 | SDL_JoystickOpen | ✅ |
| 21-22 | InputDevice(3, type=5) → joystick2 | SDL_JoystickOpen | ✅ |
| 23 | RegKey_Close | N/A | ⬜ |
| 25 | vtable[0xa0]() = ShowWindow/MainMenu | ui_render_title() | ✅ |
| 26 | — | load_assets() (our addition) | ✅ |

## App_Run Game Loop (0x46BD80)

| Original | Our Equivalent | Status |
|----------|----------------|--------|
| PeekMessage → DispatchMessage | SDL_PollEvent | ✅ |
| Scene_Update | physics_update(dt) | ✅ |
| Scene_Render → BeginScene | glClear + glMatrixMode set | ✅ |
| Scene_RenderAllObjects | render_level_objects + render_ball | ✅ |
| Scene_RenderScoreHUD | ui_render_hud | ✅ |
| Graphics_PresentOrEnd → EndScene + Present | SDL_GL_SwapWindow | ✅ |
| Sleep(targetFrameTime - elapsed) | SDL_Delay | ✅ |
| Target: 33ms/frame (~30fps) | 1000/30 = 33ms | ✅ |

## App_Shutdown (0x46DB10)

| Original | Our Equivalent | Status |
|----------|----------------|--------|
| BASS_Free | Mix_CloseAudio | ✅ |
| DirectInput8_Release | N/A (SDL handles) | ✅ |
| Graphics_Release | glDeleteTextures + SDL_GL_DeleteContext | ✅ |
| DestroyWindow | SDL_DestroyWindow | ✅ |
| UnregisterClassEx | SDL_Quit | ✅ |

## Key Constants (from binary data section)

| Address | Name | Value | Usage |
|---------|------|-------|-------|
| 0x4CF4C0 | FRICTION | 0.95f | Per-frame velocity damping |
| 0x4CF434 | Y_DAMP | 0.8f | Vertical bounce damping |
| 0x4CF4B8 | SPEED_FRICTION | 0.99f | Continuous velocity decay |
| Ball+0x284 | Ball radius | 35.0f (0x420C0000) | Collision sphere |
| Ball+0x188 | max_speed | 5000.0f | Velocity clamp |

## Status Key
- ✅ Implemented and matching original behavior
- ⏳ Partially implemented / placeholder
- ⬜ Not yet implemented