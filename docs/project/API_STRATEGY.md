# Reimplementation API Strategy

## Core Principle: Same APIs as Original

The goal is to **open-source Hamsterball**, not rewrite it with different APIs.
Every call must match what the original binary does:

- **Direct3D 8** (not OpenGL/SDL) for rendering
- **DirectInput8** (not SDL input) for keyboard/mouse/joystick
- **BASS.dll** (shipped alongside, like original) for audio
- **Win32 API** (not SDL window) for window management

Original binary is PE32 i386 (32-bit), so our build targets 32-bit Windows.
Wine handles D3D8→OpenGL translation for Linux users automatically.

## WinMain Flow (0x4278E0)

```
WinMain → RegisterClassExA("AthenaWindow") → CreateWindowExA(800,600)
  → App_Initialize_Full(0x429530)
    → App_Initialize(0x46BB40): DirectInput8Create, BASS_Init, D3D8 creation
    → LoadCursorA("BLANKCURSOR")
    → SetDisplayMode(800,600) via graphics vtable
    → SetRenderState(D3DRS_LIGHTING=TRUE, SHADEMODE=3)
    → Graphics_FindOrCreateTexture("shadow.png")
    → MusicChannel_LoadAndAppend("music\\music.mo3")
    → LoadJukebox("jukebox.xml")
    → RegKey_ReadDword("PlayCount", default=20)
    → 4x InputDevice_SetType (kb=1, mouse=2, joy1=4, joy2=5)
  → App_Run(0x46BD80): game loop (PeekMessage/TranslateMessage/DispatchMessage)
  → App_Shutdown(0x46DB10)
```

## Build Target

- **Compiler**: i686-w64-mingw32-gcc (32-bit Windows)
- **Link**: -ld3d8 -ldinput8 -ldsound -ldxguid -lwinmm -lole32 -loleaut32
- **BASS.dll**: Shipped alongside exe (same as original — it's a third-party lib, not game code)
- **D3D8**: System DLL on Windows, Wine provides it on Linux

## D3D8 Calls to Replicate (from Ghidra)

App_Initialize (0x46BB40):
  Direct3DCreate8()
  IDirect3D8_CreateDevice(D3DDEVTYPE_HAL, ...)
  SetRenderState(D3DRS_LIGHTING, TRUE)
  SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD=3)

## DirectInput8 Calls

DirectInput8Create(hInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8A, ...)
CreateDevice(GUID_SysKeyboard)
CreateDevice(GUID_SysMouse)
EnumDevices(DI8DEVCLASS_GAMECTRL, ...)  // for joysticks

## BASS Audio Calls

BASS_Init(-1, 44100, 0, 0, NULL)  // default device, 44.1kHz
BASS_StreamCreateFile(FALSE, "music\\music.mo3", ...)
BASS_ChannelPlay(stream, TRUE)