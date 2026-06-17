# 05 - Struct Offset Verification Methodology

Every documented offset in this project is guilty until proven innocent by raw decompiled C. Human-written comments can be wrong.

## 1. Confidence Markers

| Marker | Meaning |
|--------|---------|
| ✅ | Verified in raw decompiled C by 2+ distinct functions |
| ⚠️ | Found in exactly 1 decompilation or only in comments |
| ❓ | Inferred from adjacent fields; no raw C evidence yet |

## 2. Automated Verification Script

A reusable Python verifier is provided at:

```bash
python3 docs/agent-knowledge/scripts/verify_struct_offsets.py Scene > /tmp/scene_verify.md
```

It decompiles the configured list of functions, extracts all `(int)this + 0xNNNN` and `param_1[0xNNN]` references, and compares them to the claimed offsets in `CLAIMED_OFFSETS`.

Template lives at `templates/verify_struct_offsets_template.py`.

## 3. Manual Spot-Check Commands

```bash
# Direct byte offset in App functions
grep -rn '(int)this + 0x184)' analysis/ghidra/decompilations/app/

# Int-indexed access (multiply by 4)
grep -rn 'param_1\[0x5' analysis/ghidra/decompilations/app/decomp_app_run.c

# Cross-object references (e.g., Ball -> Scene)
grep -rn '0x5dc\|0x5DC' analysis/ghidra/decompilations/ball/
```

## 4. Common Offset Confusion Patterns

| Pattern | Example | Fix |
|---------|---------|-----|
| Int-indexed vs byte | `param_1[0x5C]` = `0x170` | Multiply index by 4 |
| Nested object flattened | `CollisionMesh+0xCA8` under Ball | Document pointer chain: `Ball+0x1A4 → CollisionMesh+0xCA8` |
| Wrong base object | `App+0x5DC = Scene*` | Raw C shows `Scene+0x5DC` player array, not App |
| Stale build | Doc says v1.0 offset; binary v1.1 moved it | Always state binary MD5 in findings |

## 5. Creating / Extending Structs in Ghidra

The GhidraMCP `create_struct` API expects a JSON array of fields:

```python
fields = [
    {"name": "vtable",       "offset": 0x000, "type": "pointer"},
    {"name": "owner_ball",   "offset": 0x010, "type": "pointer"},
    {"name": "roll_friction","offset": 0xC64, "type": "float"},
]
# send to create_struct endpoint
```

**Important:** `add_struct_field` works for extending existing structs. `import_data_types` is not implemented. `create_struct` requires the JSON array format shown above; key/value and comma-separated field formats are broken.

## 6. Plate Comments for ECX `this`-Param Retyping

Ghidra cannot retype register-based `this` parameters via the API (`set_parameter_type` fails on ECX). Workaround: use plate comments on functions to document intended types, or manually retype in the Ghidra GUI.
