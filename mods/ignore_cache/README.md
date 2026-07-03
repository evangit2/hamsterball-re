# Ignore Cache Mod

Forces Hamsterball to **always load `.MESHWORLD` and `.MESH` text files**, ignoring any existing `.cached` binary files. Also **prevents the game from generating new `.cached` files**.

## Problem This Solves

When you swap a `.MESHWORLD` file to test a custom level, the game may still load the old `.cached` binary version — silently ignoring your changes. The standard fix is to manually delete all `*.cached` files before each test run. This mod eliminates that step entirely.

## How It Works

The game has two code paths for loading level/mesh data:

| Path | Function | Format | Speed |
|------|----------|--------|-------|
| Cached | `LoadMeshWorld` (0x45DE30) / `LoadMesh` (0x471750) | Binary `.cached` | Fast |
| Text | `MeshWorld_Parse` (0x470930) | Text `.MESHWORLD` / `.MESH` | Slower |

Normally the game checks if a `.cached` file exists via `_check_file_access` (GetFileAttributesA). If it does, it loads the binary directly. If not, it parses the text file and writes a new `.cached` for next time.

This mod patches **3 locations**:

1. **`LoadMeshWorld` at 0x45DE77**: `JNZ → JMP` — always take the text-parse path, skip `.cached` load for level files
2. **`LoadMesh` at 0x4717DB**: `JNZ → JMP` — same, for mesh files in `Meshes/` folder
3. **`Mesh_SaveAndFree` at 0x46F67D**: `JZ → JMP` — skip the write block, never generate `.cached` files

## Installation

1. Open Cheat Engine, attach to `Hamsterball.exe`
2. Load `IgnoreCache.CEA` as a script
3. Enable it

No DLL, no threads, no IAT hooks — just 3 byte patches.

## Performance Impact

Levels will load slightly slower since the text parser runs every time instead of the binary loader. On modern hardware this is negligible (sub-second even for the largest 1MB levels). On 2004-era hardware it would be more noticeable.

## Files

- `IgnoreCache.CEA` — Cheat Engine AutoAssembler script
