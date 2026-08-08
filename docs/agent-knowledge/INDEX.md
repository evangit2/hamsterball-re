# Agent Knowledge Package - Hamsterball Reverse Engineering

This folder is a self-contained bootstrapping guide for any future agent that needs to reproduce or continue the Hamsterball RE work.

Start here and read the numbered sections in order. Each file is designed to be actionable on its own.

| # | File | Purpose |
|---|------|---------|
| 1 | [01-BOOTSTRAP.md](01-BOOTSTRAP.md) | Set up workspace, acquire assets, run first binary analysis |
| 2 | [02-GHIDRA-SETUP.md](02-GHIDRA-SETUP.md) | Import binary, start GhidraMCP headless, verify server |
| 3 | [03-RENAME-RESTORE.md](03-RENAME-RESTORE.md) | Import the 975+ function renames from FUNCTION_MAP.md |
| 4 | [04-DECOMP-WORKFLOW.md](04-DECOMP-WORKFLOW.md) | How to decompile, clean, and verify code faithfully |
| 5 | [05-STRUCT-VERIFICATION.md](05-STRUCT-VERIFICATION.md) | Methodology for confirming struct offsets |
| 6 | [06-MODDING-PATTERNS.md](06-MODDING-PATTERNS.md) | Common game RE patterns (dead code, vtables, level gating) |
| 7 | [07-REIMPL-LESSONS.md](07-REIMPL-LESSONS.md) | All lessons learned from reimplementation attempts |
| 8 | [08-TROUBLESHOOTING.md](08-TROUBLESHOOTING.md) | Known failure modes and workarounds |
| 9 | [scripts/](scripts/) | Reusable Python helpers |
| 10 | [templates/](templates/) | Copy-paste decompile/verify script templates |

## Topic-Deep-Dives

| File | Purpose |
|------|---------|
| [dizzy-stun-system.md](dizzy-stun-system.md) | Complete analysis of the dizzy/stun/trajectory system: respawn stun, bounce-induced dizzy counter, two-pass collision architecture, end-screen "DIZZIED BALLS:" counter, E:NODIZZY (unrelated), per-player data blocks, all physics constants |
| [death-pending-flag-deep-dive.md](death-pending-flag-deep-dive.md) | Ball+0x2E9 (death_pending) lifecycle: all 8 events that set it, reset paths, disassembly proof |
| [ball-ground-detection.md](ball-ground-detection.md) | Ball ground contact, falling detection, respawn point selection |
| [collision-system-analysis.md](collision-system-analysis.md) | Collision detection pipeline, SpatialTree, collision entry struct layout |
| [race-start-hooking.md](race-start-hooking.md) | Race lifecycle: countdown, start, finish, scoring hooks |
| [A1-THISCALL.md](A1-THISCALL.md) | __thiscall calling convention deep-dive for Ghidra decompilation |
| [A2-OBJECT-SPAWNING.md](A2-OBJECT-SPAWNING.md) | Runtime object spawning: operator_new, ctor, AthenaList_Append patterns |
| [A3-CAMERA-PITFALLS.md](A3-CAMERA-PITFALLS.md) | Camera system: 5 modes, pitch hardcode, smooth distance per-frame override |
| [../SWEAT_MODE.md](../SWEAT_MODE.md) | Sweat bubble physics assist: ball+0x260 flag, ArenaBoard toggle timer, min slope speed 0.25, flashing visual |
| [../gameplay/MAGNIFIER_HEAT_SYSTEM.md](../gameplay/MAGNIFIER_HEAT_SYSTEM.md) | Sky Race magnifying glass: heat counter (ball+0xC50), burning flag (ball+0xC58), white→red color shift via Graphics_SetColorMultiplier (0x00401160, NOT a viewport clip) |
| [TIME_TRIAL_GHOST_SYSTEM.md](TIME_TRIAL_GHOST_SYSTEM.md) | Time Trial ghost: per-frame BallSnapshot recording/playback, BestTimeTracker struct, ghost ball creation in Board_ctor, why ghost disappears on quit |
| [race-previews-and-race-name-table.md](race-previews-and-race-name-table.md) | Race previews: practice-*.png sprites in PracticeMenu (0x42EA30, slots +0xCDC..+0xCFC), tourney-*.png icon strip preloaded to App+0x3B4..0x3EC and drawn 128x128 @ 155px spacing by TourneyMenu_Render (0x450AF0, loop 0x450C46); 16-entry race-name table @ 0x4F7080 (index 15 = "YOU FOUND A BUG RACE", reachable only via unbounded accessor 0x4264A0 from "NEXT UP:" banner 0x451430) |

## External Authorities

- The repo's `docs/FUNCTION_MAP.md` is the source of truth for named functions.
- The repo's `analysis/ghidra/structs/*.h` contain C struct definitions exported from Ghidra.
- `docs/STRUCTS_AND_TYPES.md` is the human-facing struct reference (but verify offsets with raw decompilation before trusting).

## First Command Checklist

```bash
cd ~/hamsterball-re
file originals/installed/extracted/Hamsterball.exe
python3 docs/agent-knowledge/scripts/check_server.py
```
