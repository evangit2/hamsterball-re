# 08 - Troubleshooting Known Failure Modes

## Ghidra / GhidraMCP

### Server returns `program_loaded: false`
1. Read `/tmp/ghidra-mcp.log`.
2. Confirm `--project` ends in `.gpr` and `--program` starts with `/`.
3. Re-import with `analyzeHeadless` if the program was never loaded.
4. After re-import, restore renames from `docs/FUNCTION_MAP.md`.

### `run_script_inline` fails with BundleHost error
Inline Python/Ghidra scripts are broken in headless mode. Use native Java scripts in `analysis/ghidra/scripts/` instead.

### MCP tools missing from Hermes
1. Check `~/.hermes/config.yaml` for the `mcp_servers: ghidra-mcp:` bridge block.
2. Verify `/opt/ghidra-mcp/bridge_mcp_ghidra.py` exists.
3. Fall back to direct `curl` GET requests.

### Decompile by name returns "Address or function name is required"
Use `?address=0x...` instead of `?name=...`. Name lookups are unreliable.

## Tools / Environment

### `pip` vs `python3` mismatch
This machine has `python3=3.11.15` with no pip module; `pip` points to python3.12. Use `uv` or a venv for Python dependencies.

### Web search / web extract hanging
Ensure `FIRECRAWL_API_KEY` and `FIRECRAWL_API_URL` are set in the Hermes config pointing to the Guanaco router (or disable web fallback).

### Guanaco router `AttributeError` (UsageConfig missing fields)
The installed `guanaco` package is older than the repo code. Fix:
```bash
cd ~/.guanaco/repo
source ~/.guanaco/venv/bin/activate
pip install -e .
```

## Binary / RE

### Spawned BadBall crashes
- Verify `__thiscall` convention.
- Verify ECX = allocated memory.
- Ensure the Scene pointer and CollisionLevel pointer at `Scene+0x440` are valid.
- Use `HeapAlloc` on the game's CRT heap, not `operator_new` from a DLL.

### Camera geometry invisible
Try both `eye = target + dir*dist` and `eye = target - dir*dist` or sample which side has vertices in front of the camera.

### Textures not rendering under Wine/llvmpipe
Programmatic textures via `CreateTexture` + `LockRect` work; loaded BMP/PNG textures may not. Use runtime-generated checker textures with `D3DTOP_MODULATE` for correct per-pixel modulation.

## Commit Safety

### Never commit
- Original binaries (`.exe`, `.dll`)
- Installer packages
- User credentials or API tokens
- Save games or registry data

### Always commit
- `docs/FUNCTION_MAP.md`
- `analysis/ghidra/renames_backup.json`
- `analysis/ghidra/structs/*.h`
- Clean C++ reference reconstructions in `analysis/code_ref/`
