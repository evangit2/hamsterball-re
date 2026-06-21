# Hamsterball - Runtime Environment

## System Requirements (from original game)

- **OS**: Windows 2000/XP/Vista/7
- **DirectX**: Version 8 or higher required
- **CPU**: Pentium II or equivalent
- **RAM**: 64MB minimum
- **Video**: DirectX 8 compatible 3D accelerator
- **Sound**: DirectX compatible sound card

## Registry Keys

The game uses ADVAPI32.dll functions:
- `RegOpenKeyExA`, `RegOpenKeyA` - Open registry keys
- `RegCreateKeyA` - Create registry keys  
- `RegQueryValueExA` - Read values
- `RegSetValueExA` - Write values
- `RegCloseKey` - Close keys

### Likely Registry Locations (CONFIRMED from eSellerate API):
- `HKLM\SOFTWARE\Raptisoft\Hamsterball` - Game installation path
- `HKLM\SOFTWARE\Microsoft\DirectX` - DirectX version check
- eSellerate DRM keys for activation

## File Paths Used

### From binary string analysis:
- `DATA\HS.CFG` - High scores and configuration save file (CONFIRMED)
- `Levels\*.MESHWORLD` - Level data files (CONFIRMED)
- `Meshes\*.MESH` - 3D model files (CONFIRMED)
- `Textures\*.png`, `Textures\*.bmp` - Texture files (CONFIRMED)
- `Sounds\*.ogg` - Sound effects (CONFIRMED)
- `Music\Music.mo3` - Background music (CONFIRMED)
- `Fonts\*\font.description` - Font metric data (CONFIRMED)
- `Fonts\*\Data0.png` - Font texture atlases (CONFIRMED)
- `%s.mesh` - Format string for mesh loading
- `%s.meshcollision` - Format string for collision mesh loading
- `%s.meshworld` - Format string for level loading
- `%s.ogg`, `%s.wav` - Format strings for sound loading
- `%s\data%d.png` - Format string for mipmapped texture loading

### Configuration Files:
- `Jukebox.xml` - Music mapping (CONFIRMED)
- `RaceData.xml` - Race timing/scoring parameters (CONFIRMED)
- `DATA\HS.CFG` - Save data with player names and high scores (CONFIRMED)
- `DATA\TOURNAMENT.SAV` - Tournament progress (CONFIRMED)

## DLL Dependencies

### Required at Runtime:
| DLL | Purpose | Notes |
|-----|---------|-------|
| d3d8.dll | DirectX 8 3D rendering | Primary graphics API |
| DINPUT8.dll | DirectInput 8 | Input handling (keyboard/gamepad) |
| DSOUND.dll | DirectSound | Audio output via DirectSound |
| BASS.dll | BASS audio library | Music playback (MO3 format) |
| KERNEL32.dll | Windows core API | File I/O, memory, threads |
| USER32.dll | Windows user interface | Window creation, message loop |
| GDI32.dll | Graphics Device Interface | Text rendering |
| ADVAPI32.dll | Windows registry | Settings storage |
| SHELL32.dll | Windows Shell | ShellExecute for URLs |
| ole32.dll | COM | COM initialization |
| WS2_32.dll | Winsock | Network for eSellerate |
| VERSION.dll | Version checking | DLL version queries |

### Optional/Conditional:
| DLL | Purpose | Notes |
|-----|---------|-------|
| d3d9.dll | DirectX 9 fallback | Referenced in strings, may be D3D8->D3D9 compat |
| d3d8d.dll | DirectX 8 debug | Debug version checking |
| eSellerateEngine.dll | DRM/activation | Third-party licensing |
| RICHED32.DLL | Rich text | Possibly for license dialogs |
| COMCTL32.dll | Common controls | UI elements |

## DirectX 8 Interfaces Used

From import analysis:
- `Direct3DCreate8` - Main D3D8 entry point (CONFIRMED)
  - Creates IDirect3D8 interface
  - Used to create IDirect3DDevice8
  - Game uses D3D8 for all 3D rendering

- DirectSound (via DSOUND.dll ordinal import)
  - Sound playback for effects

- DirectInput8 (via DirectInput8Create)
  - Keyboard and mouse input at minimum
  - Possibly gamepad/joystick support

## BASS Audio Library Usage

```
BASS_Init()          - Initialize audio system
BASS_Start()         - Start audio playback
BASS_Stop()          - Stop all audio
BASS_Free()          - Free audio resources
BASS_SetConfig()     - Configure audio settings
BASS_ErrorGetCode()  - Get last error
BASS_MusicLoad()     - Load MO3 music module
BASS_MusicPlayEx()   - Play music with position control
BASS_ChannelSetAttributes() - Set volume/pan/rate
BASS_ChannelStop()   - Stop specific channel
```

## Wine Execution Testing

### Status: PARTIALLY WORKING
- Wine 9.0 initializes and launches the process
- No display server available for actual rendering
- Audio ALSA errors (no sound card in sandbox)
- Process stays running, indicating game loop starts

### Next Steps for Wine Testing:
1. Set up X virtual framebuffer (Xvfb)
2. Install DirectX 8 runtime via winetricks
3. Test with: `xvfb-run wine Hamsterball.exe`
4. Consider using Wine with D3D shader converter (dxvk)
5. Document any crashes or error dialogs

## eSellerate DRM

The game contains eSellerate activation code:
- Purchase dialog strings present
- Network activation via eSellerate Engine
- Free play counter (shareware/trial version)
- "CLICK HERE TO BUY!" / "CLICK HERE TO REGISTER" strings
- For reimplementation: skip DRM entirely (we have permission)

## Window Configuration

From WinMain analysis:
- Window class registered via RegisterClassExA / RegisterClassA
- Window created via CreateWindowExA
- Uses PeekMessageA for game loop (non-blocking message pump)
- SetCursor/ShowCursor for cursor management
- SetCapture/ReleaseCapture for mouse capture
- ClientToScreen/ScreenToClient for coordinate conversion
- Game window likely uses WS_EX_APPWINDOW style