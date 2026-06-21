# Hamsterball Browser D3D8 Shim — Phase 1: API Verification

## Binary
- Path: `originals/installed/extracted/Hamsterball.exe`
- Size: 1,404,928 bytes
- MD5: `7d25019366b8d7f55906325bd630d7fe`
- Type: PE32 i386 executable

## Import Verification

### DirectX Graphics API
- **Imports `d3d8.dll`** — confirmed via `objdump -p`
- **Only one D3D8 import**: `Direct3DCreate8`
- All other D3D8 calls go through COM interface vtables (IDirect3D8, IDirect3DDevice8, etc.)
- No `d3d9.dll`, `ddraw.dll`, or `dxgi.dll` imports

### Dead Code Strings
- `d3d9.dll`, `d3d8d.dll`, `D3D9` — present as strings but NOT imported
- These are dead code in the Athena engine (optional API path compiled out)

### Other Graphics Dependencies
- `GDI32.dll` — standard Win32 GDI (window, font, bitmap)
- `USER32.dll` — window management, input messages
- `SHELL32.dll` — shell operations

### Input
- `DINPUT8.dll` — DirectInput 8 keyboard input

### Audio
- `BASS.dll` — BASS audio library (music/sound)
- `DSOUND.dll` — DirectSound for sound effects

### Networking
- `WS2_32.dll` — Winsock (eSellerate DRM/registration)
- `eSellerateEngine.dll` — DRM/licensing

## Conclusion
Hamsterball is a **fixed-function Direct3D 8** game. It uses:
- `Direct3DCreate8` → COM interfaces
- No shaders (D3D8 fixed-function pipeline)
- DrawPrimitiveUP for rendering
- DirectInput for input
- BASS + DirectSound for audio

No D3D9 path exists at runtime. The shim must implement D3D8 COM interfaces.
