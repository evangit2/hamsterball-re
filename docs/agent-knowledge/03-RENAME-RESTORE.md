# 03 - Restore Function Renames from FUNCTION_MAP.md

The Hamsterball project has 975+ hand-labeled functions. The authoritative list lives in `docs/FUNCTION_MAP.md`. This file explains how to apply those names to a fresh Ghidra project.

## 1. Prerequisite

- GhidraMCP headless server running on port 8089
- Program loaded (`program_loaded: true` on `/health`)

## 2. Run the Restore Script

```bash
cd ~/hamsterball-re
python3 analysis/ghidra/apply_renames.py
```

The script:
1. Parses `docs/FUNCTION_MAP.md` for `| 0x... | Name |` rows.
2. Skips auto-generated names (`Catch`, `Unwind`, `operator_`, `thunk_`, `entry`, `Start`).
3. Sends batches of 50 names to `http://127.0.0.1:8089/batch_create_labels`.
4. Prints progress and final documentation percentage.

## 3. Verify Coverage

```bash
curl -s http://127.0.0.1:8089/compare_programs_documentation | python3 -m json.tool
```

Expected final coverage: **~100%** (3,781/3,781 functions documented).

## 4. Export Renames Back to JSON (After Heavy RE Session)

If you add new labels, back them up:

```bash
cd ~/hamsterball-re
python3 - <<'PY'
import json, re
renames = []
with open('docs/FUNCTION_MAP.md', 'r') as f:
    for line in f:
        m = re.match(r'\|\s*(0x[0-9a-fA-F]+)\s*\|\s*(\S+.*?)\s*\|', line.strip())
        if m:
            addr, name = m.group(1), m.group(2).strip()
            if name not in ('Address', 'Name', '---'):
                renames.append({'address': addr, 'name': name})
with open('analysis/ghidra/renames_backup.json', 'w') as f:
    json.dump({'renames': renames, 'count': len(renames),
               'timestamp': '2026-06-17', 'source': 'FUNCTION_MAP.md'}, f, indent=2)
print(f"Backed up {len(renames)} renames")
PY
```

## 5. Maintaining FUNCTION_MAP.md

When you discover and rename a new function:

1. Add it to `docs/FUNCTION_MAP.md` in the appropriate subsystem section.
2. Run `apply_renames.py` to sync Ghidra.
3. Commit both `docs/FUNCTION_MAP.md` and `analysis/ghidra/renames_backup.json`.

## 6. Format Rules for FUNCTION_MAP.md

```markdown
## Subsystem Name

| Address | Name | Description |
|---------|------|-------------|
| 0x004XXXXX | FunctionName | What it does |
```

- Address `0x` prefix, 8 hex digits.
- Name is one word (no spaces), snake_case allowed.
- Description is concise but specific.
