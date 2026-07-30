# plus_ghost_event — E:GHOST Custom Event Mod (HB+)

HB+ port of the `ghost_event` bass.dll proxy mod.

## What it does

Creates a custom collision event `E:GHOST(filename)` that loads a `.ghost`
file and plays it back as a ghost ball in **any game mode** (Time Trial,
Tournament, or Party).

When the ball touches a mesh surface tagged `E:GHOST(PreviousRun)`:
1. Loads `Ghosts/PreviousRun.ghost` from the directory containing the DLL
2. Creates a BestTimeTracker from the ghost data
3. Injects it into `App+0x910` (playback buffer)
4. Creates a ghost ball at `board+0x361C` if one doesn't exist
5. The game's own `Level_UpdateAndRender` renders the ghost ball every frame
   (the TT/party checks have been NOP'd out via `PatchMemory`)
6. The mod advances the playback index every frame via `onGameUpdate`

## Usage

1. Place `.ghost` files in a `Ghosts/` directory next to the mod DLL.
   - Files can be created by the ghost_saver mod (Time Trial mode).
   - Binary format: magic `"GHSG"` + version + time + frameCount +
     frameCount × 0x28-byte snapshot entries.
2. Add `E:GHOST(filename)` event tags to level MESHWORLD files
   (Section 1 entity names, or MeshBuffer names).
3. When the ball touches the tagged surface, the ghost starts playing.

## Example

```
E:GHOST(PreviousRun)  -> loads Ghosts/PreviousRun.ghost
E:GHOST(Beginner)     -> loads Ghosts/Beginner.ghost
```

## Installation

1. Copy `plus_ghost_event.dll` into your HB+ `Mods/` folder.
2. Create a `Ghosts/` directory next to the DLL and place `.ghost` files there.
3. Enable the mod in the HB+ mod menu.

## Files

- `GhostEventMod.cpp` — Source code
- `plus_ghost_event.dll` — Compiled HB+ mod
- `README.md` — This file
