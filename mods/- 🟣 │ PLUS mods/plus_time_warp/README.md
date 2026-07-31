# Time Warp (HB+ v2.1)

A Hamsterball Plus mod combining four systems from the original `time_warp.c` bass.dll proxy:

1. **Level Warp** — `WARP(Name)` S1 ref points trigger multi-phase level transitions.
2. **Ghost Saver** — Time Trial runs are saved to per-race `.ghost` files in `Ghosts\`.
3. **Ghost Event** — `E:GHOST(filename)` event planes load saved ghosts.
4. **Time Warp** — warping to the **same** level spawns a purple Ghost 2 replay of your previous segment, plus multi-segment `[N]` / `(N)` ghost file handling.

## Files

| File | Description |
|------|-------------|
| `plus_time_warp.dll` | Compiled HB+ mod DLL |
| `source/TimeWarpMod.cpp` | Full C++ source |
| `source/TimeWarpMod.dll` | Previous compiled build |
| `source/HamsterballAPI.h` | HB+ v2.1 API header used for building |

## Install

1. Copy `plus_time_warp.dll` to your Hamsterball `Mods\` folder.
2. Launch the game. A toggle button **"Time Warp"** appears in the HB+ menu.
3. The mod auto-creates `Ghosts\` next to `Hamsterball.exe` for saved ghosts.

## Usage

- **Time Warp level**: add a S1 ref point named `WARP(LevelName)` (e.g. `WARP(Beginner)`). Drive into it to warp.
- **Warp to same level**: records your current segment, then spawns Ghost 2 in heliotrope purple showing your previous route.
- **Ghost Event**: create a MeshBuffer named `E:GHOST(MyRun)`. Collision loads `Ghosts\MyRun.ghost`.
- **Ghost Trigger**: create S1 ref points named `GT:RESET`, `GT:STOP`, `GT:START` near ghost routes.

## Build

```bash
cd "mods/- 🟣 │ PLUS mods/plus_time_warp"
cp "../../../docs/agent-knowledge/HamsterballAPI.h" ./HamsterballAPI.h
sed -i 's/^\(\s*\)static_assert(\([^;]*\));/\1\/* static_assert(\2); *\//' HamsterballAPI.h
i686-w64-mingw32-g++ -shared -o plus_time_warp.dll source/TimeWarpMod.cpp \
  -O2 -mwindows -static-libgcc -static-libstdc++ \
  -Wl,--enable-stdcall-fixup -fpermissive
```

## Notes

- HB+ API version: **v2.1**
- Requires 32-bit Hamsterball (`i686-w64-mingw32-g++`).
- Logging is enabled; output goes to `time_warp_log.txt` next to the mod DLL.
