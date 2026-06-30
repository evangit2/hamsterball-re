# Hamsterball — Reverse Engineering Experiments

A collection of reverse-engineering experiments targeting **Hamsterball** (2004, Raptisoft) —
a 3D marble racing game built on the Athena engine (PE32 i386, DirectX 8 / BASS audio / DirectInput8).

This project explores the game's internal architecture, binary file formats, physics systems,
and modding capabilities through Ghidra decompilation, DLL proxy mods, and custom level creation.

---

## What's in this repo

> 🗺️ **[Interactive Knowledge Map](https://evangit2.github.io/hamsterball-re/)** — browse all 183 documents as an interactive graph with full-text search (Ctrl+K).

### 📖 Documentation (81 files across 11 categories)

Comprehensive reverse-engineering documentation generated from Ghidra decompilation of the
original `Hamsterball.exe`, covering every major game subsystem:

| Category | Docs | Highlights |
|---|---|---|
| `objects/` | 15 | App, Ball, Scene struct layouts, object factory, global variables |
| `physics/` | 13 | Collision system, ball physics, event planes, raycasting, particle system |
| `meshworld/` | 7 | Binary file format spec, object types, parser decompilation |
| `rendering/` | 7 | D3D8 pipeline, camera system, font/text rendering, iteration logs |
| `gameplay/` | 7 | Arena scoring, rumble board, 8-ball AI, tournament state machine |
| `decompilation/` | 7 | Key function decompilations, full function map, game loop analysis |
| `modding/` | 6 | DLL modding guides, function reference, custom controls, audio modding |
| `project/` | 12 | Build notes, file formats, registry system, asset manifest |
| `ui/` | 3 | Menu system, HUD/timer, text elements |
| `audio/` | 2 | Audio system, SFX reference |
| `input/` | 2 | DirectInput, full input system with control remapping |

Plus `docs/agent-knowledge/` — a structured onboarding guide for AI agents working on the codebase.

### 🛠️ DLL Mods (15+ mods)

Runtime modifications via `bass.dll` proxy injection — no game patches required:

- **player_clones** — Spawn AI-controlled player balls with custom targeting
- **jump_mod** — Jump physics with ground detection via raycasting
- **8ball_hit_detect** — Detect and log 8-ball collisions
- **entity-limit-fixer** — Fix game crashes from too many entities
- **fps_unlock** — Uncap the 30 FPS framerate limit
- **half_size_balls** — Shrink ball collision radius
- **water_mod** — Water surface rendering experiment
- **collision_hook** — Intercept ball-ball collision events
- **unlimited_tris** — Remove triangle render limit

...and more. See [`mods/README.md`](mods/README.md) for the full catalog.

### 🎮 Custom Levels

Custom `.MESHWORLD` level files created from scratch, verified working in the original game:

- `DualPlatformArena.MESHWORLD` — Two-platform arena layout
- `DualPlatformArenaV2.MESHWORLD` — Refined version with railings

### 🔧 Tools

- `tools/mw_create.py` — Python MESHWORLD level generator
- `tools/d3d8_proxy_logger/` — D3D8 COM proxy logger for API call tracing
- `tools/recon-analyzer/` — Automated binary reconnaissance
- `tools/decompile_batch.py` — Batch Ghidra decompilation
- `tools/hbtestd/` — Automated game testing harness

### 📊 Binary Analysis

- `analysis/` — Function catalogs, struct layouts, and JSON exports from Ghidra
- `reference/raptisoft-exporter/` — Raptisoft's official MESHWORLD exporter reference

---

## Technical Details

**Target binary:** `Hamsterball.exe` (PE32 i386, ~580KB)

**Engine:** Athena engine — DirectX 8 (D3D8), BASS audio library, DirectInput8

**Analysis tools:** Ghidra (with custom MCP server for programmatic decompilation),
Cheat Engine for runtime hooks, MinGW for cross-compiling DLL proxies.

**Key addresses:**

| Symbol | Address |
|---|---|
| App global | `0x005341E0` |
| WinMain | `0x004278E0` |
| Game loop | `0x0046BD80` |
| Ball vtable | `0x004CF3A0` |
| Scene vtable | `0x004D0260` |

## Repo Structure

```
docs/           # 81 RE docs in 11 category subfolders
mods/           # 15+ compiled DLL mods with source
tools/          # Python/C tools for analysis and level creation
analysis/       # JSON catalogs and struct exports from Ghidra
reference/      # Raptisoft official exporter reference
*.MESHWORLD     # Custom level files
```

## Credits

**RodentRacer** — Contributions to the project

**Artizard** — Contributions to the project

**BookwormKevin** — Contributions to the project

**Makyuni** — Contributions to the project

**XRow** — Contributions to the project

## License

Analysis and mod code is original work. Original Hamsterball is copyright Raptisoft.
No original game binaries are distributed in this repo.
