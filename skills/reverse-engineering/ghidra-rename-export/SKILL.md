---
name: ghidra-rename-export
description: Export all current Ghidra renames to JSON backup in git repo. Run after every RE session.
version: 2026-04-14
---

# Ghidra Rename Export (Post-Session Backup)

**MUST run after every RE session** to persist renames outside the Ghidra project DB.

## Quick Export (HTTP API — slow, use as fallback)

The HTTP API paginating through 3781 functions at 100/page **times out even at 300s** due to per-request latency. Do NOT rely on this as primary method.

## Primary Export: Parse FUNCTION_MAP.md

FUNCTION_MAP.md is the source of truth. Always up to date. Parse it directly:

```bash
cd ~/hamsterball-re && python3 -c "
import json, re
renames = []
with open('docs/FUNCTION_MAP.md', 'r') as f:
    for line in f:
        m = re.match(r'\|\s*(0x[0-9a-fA-F]+)\s*\|\s*(\S+.*?)\s*\|', line.strip())
        if m:
            addr, name = m.group(1), m.group(2).strip()
            if name not in ('Address', '---', '---------', 'Name'):
                renames.append({'address': addr, 'name': name})
with open('analysis/ghidra/renames_backup.json', 'w') as f:
    json.dump({'renames': renames, 'count': len(renames), 'timestamp': '2026-04-14', 'source': 'FUNCTION_MAP.md'}, f, indent=2)
print(f'Exported {len(renames)} function entries')
"
```

## Fallback: MCP tool pagination (batch of 100)

The `list_functions_enhanced` MCP tool works but is capped at 100/page and slow. Use for spot checks only:

```
list_functions_enhanced offset=0 limit=100 → save
list_functions_enhanced offset=100 limit=100 → append
...
```

## Inline Script Runner is BROKEN

`run_script_inline` fails with:
```
GhidraScriptLoadException: BundleHost.getBundleFiles() is null
```
The headless GhidraMCP server cannot execute JavaScript or Python inline scripts. **Do not attempt** inline scripts — they will not work.

## Ghidra Python Scripts (`run_script`) also broken

Ghidra was not started with PyGhidra. Python scripts (`*.py`) are not available.
Only `*.java` scripts can run, and only if the BundleHost issue is resolved.
Workaround: use `list_functions_enhanced` MCP tool instead.

## Also Update FUNCTION_MAP.md

After each RE session, update the function map docs:
1. Add new function entries to `docs/FUNCTION_MAP.md`
2. Commit & push changes

## Git Commit

```bash
cd ~/hamsterball-re
git add analysis/ghidra/renames_backup.json docs/FUNCTION_MAP.md
git commit -m "Session N: backup renames (X% documented)"
git push
```

## Why This Matters

The Ghidra project DB (`.rep/` directory) is NOT version controlled. If it gets corrupted or re-imported, ALL renames are LOST unless exported to git. This happened in April 2026 and cost us ~14% documentation progress (75.5% -> 61%).