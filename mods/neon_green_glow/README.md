# Neon Green Glow

Changes the Neon Race player ball and platform glow from **yellow** to **green**.

## How it works

`Scene_SetupLevelDark` (0x00416270) writes RGBA color values to the ball physics struct's material fields using `PUSH imm32` instructions. Each R-channel push loads `1.0f` (0x3F800000) to set the red component to full.

This patch zeroes all 8 R-channel push immediates, changing the color from:
- **Original:** RGBA = (1.0, 1.0, 0.0, 1.0) = **yellow**
- **Patched:** RGBA = (0.0, 1.0, 0.0, 1.0) = **green**

## Patched addresses

| Address | Original | Patched | Target |
|---------|----------|---------|--------|
| 0x00416360 | 3F800000 | 00000000 | Loop: platform colors R |
| 0x004163D4 | 3F800000 | 00000000 | Loop: platform colors R |
| 0x00416568 | 3F800000 | 00000000 | P1 Ambient R (phys+0x1CC) |
| 0x004165D5 | 3F800000 | 00000000 | P1 Diffuse R (phys+0x1BC) |
| 0x0041663F | 3F800000 | 00000000 | P1 Emissive R (phys+0x1EC) |
| 0x004166C0 | 3F800000 | 00000000 | P2 Ambient R (phys2+0x1CC) |
| 0x0041672D | 3F800000 | 00000000 | P2 Diffuse R (phys2+0x1BC) |
| 0x00416797 | 3F800000 | 00000000 | P2 Emissive R (phys2+0x1EC) |

## Usage

1. Open Cheat Engine, attach to Hamsterball.exe
2. Load `NeonGreenGlow.CEA`
3. Enable before entering Neon Race
4. The glow will be green when the level loads

## DLL Version (bass.dll proxy)

A `bass.dll` proxy version is also included. No Cheat Engine required.

### Installation
1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` into the game folder
3. Launch Hamsterball — the glow will be green on Neon Race

### How it works
At DLL load time, `apply_patches()` uses `VirtualProtect` to make the `.text` section writable, then writes `0x00000000` (0.0f) over each of the 8 R-channel `PUSH` immediates. The original `bass_real.dll` is loaded for audio passthrough.

### Crash test
Tested via hbtestd on Wine/Xvfb: 38.8s runtime, no crash, verdict OK.

## Verified

- Tested on Wine/llvmpipe via hbtestd
- Patch confirmed: neon platforms changed from yellow to green
- Original bytes restored after testing
- DLL crash test: 38.8s, no crash
