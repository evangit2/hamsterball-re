# 04 - Decompilation and Code-Reference Workflow

## 1. Single Address Decompile via REST

```bash
ADDR=0x0046BD80  # App_Run
curl -s "http://127.0.0.1:8089/decompile_function?address=$ADDR"     > analysis/code_ref/decomp_app_run.c
```

## 2. Batch Decompile via Headless Script

When the REST server is slow or unreliable, use a native Ghidra Java script. A template is provided at:

```
analysis/ghidra/scripts/DecompileByAddress.java
```

Run it from the repo root:

```bash
/opt/ghidra/support/analyzeHeadless \
  $(pwd)/analysis/ghidra/HamsterballProject Hamsterball \
  -scriptPath $(pwd)/analysis/ghidra/scripts \
  -postScript DecompileByAddress.java 0x46EC30 /tmp/decomp_46EC30.c
```

For multiple addresses, use `DecompileMultiAddresses.java` (same directory) with a comma-separated address list.

## 3. Cleaning Decompiled Code

When converting raw Ghidra C to readable reference code:

DO:
- Give variables meaningful names.
- Replace magic numbers with named constants.
- Add section comments (`// === physics integration ===`).
- Preserve original pointer arithmetic semantics (use `PTR_OFF`, `READ_U8`, `WRITE_U8` macros).

DO NOT:
- Replace flat `__strnicmp` chains with `switch`/`enum class`.
- Change `__thiscall` free functions into C++ class methods with early returns.
- Factor repeated pointer arithmetic into accessor methods unless the original did so.

See `analysis/code_ref/CreateNoDizzy_clean.cpp` for an example of a faithful cleanup.

## 4. Required Reading for New Findings

After decompiling a new function, update these canonical docs:

| If you found... | Update... |
|-----------------|-----------|
| New function | `docs/FUNCTION_MAP.md` |
| New field / struct | `docs/STRUCTS_AND_TYPES.md` + `analysis/ghidra/structs/*.h` |
| Modding behavior | Relevant `docs/*_MODDING.md` or create new file |
| Rendering behavior | `docs/D3D8_RENDERING_PIPELINE.md` or `docs/RENDERING_ITERATION_LOG.md` |
| Input / controls | `docs/INPUT_SYSTEM.md` |

## 5. Naming Conventions

| Construct | Convention | Example |
|-----------|------------|---------|
| Functions | `Subsystem_VerbNoun` | `Ball_ApplyForce`, `Scene_SetCamera` |
| Structs | PascalCase | `Ball`, `Scene`, `CollisionMesh` |
| Offsets | `UPPERCASE` with prefix | `BALL_POS_X`, `SCENE_BALL_LIST` |
| Globals | `g_` prefix | `g_app`, `g_graphics` |

## 6. Cross-Reference Search Patterns

```bash
# Find all references to a byte offset in raw decompilations
grep -rn '(int)this + 0xCA8)' analysis/ghidra/decompilations/
grep -rn 'param_1\[0x32a\]' analysis/ghidra/decompilations/   # 0x32a * 4 = 0xCA8

# Find xrefs to a vtable
grep -rn '0x4CF3A0' analysis/code_ref/ docs/ analysis/ghidra/structs/
```
