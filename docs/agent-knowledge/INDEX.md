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
| [limit-flag-deep-dive.md](limit-flag-deep-dive.md) | Ball+0x2E9 (dizzy_lock) lifecycle: all 8 events that set it, reset paths, disassembly proof |
| [ball-ground-detection.md](ball-ground-detection.md) | Ball ground contact, falling detection, respawn point selection |
| [collision-system-analysis.md](collision-system-analysis.md) | Collision detection pipeline, SpatialTree, collision entry struct layout |
| [race-start-hooking.md](race-start-hooking.md) | Race lifecycle: countdown, start, finish, scoring hooks |
| [A1-THISCALL.md](A1-THISCALL.md) | __thiscall calling convention deep-dive for Ghidra decompilation |
| [A2-OBJECT-SPAWNING.md](A2-OBJECT-SPAWNING.md) | Runtime object spawning: operator_new, ctor, AthenaList_Append patterns |
| [A3-CAMERA-PITFALLS.md](A3-CAMERA-PITFALLS.md) | Camera system: 5 modes, pitch hardcode, smooth distance per-frame override |
| [../SWEAT_MODE.md](../SWEAT_MODE.md) | Sweat bubble physics assist: ball+0x260 flag, ArenaBoard toggle timer, min slope speed 0.25, flashing visual |

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
