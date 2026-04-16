---
name: ghidra-rename-backup
description: Backup and restore Ghidra function/label renames to/from a JSON file in the git repo
version: 2026-04-13
---

# Ghidra Rename Backup & Restore

**CRITICAL**: Always backup renames after a RE session. Restoring from backup is the ONLY way to recover if the Ghidra project DB gets corrupted or re-imported.

## Architecture

- Renames are exported to `~/hamsterball-re/analysis/ghidra/renames.json`
- Format: `{"renames": [{"address": "00401234", "old_name": "FUN_00401234", "new_name": "Ball_Update"}], ...}`
- This file is committed to git so renames survive project DB loss

## Export Renames (run after each session batch)

```bash
# The export is done via GhidraMCP - run the inline script
# Or use the MCP batch_create_labels tool
```

### Export via Inline Ghidra Script

Use `mcp_ghidra_mcp_run_script_inline` with this Java code:

```java
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
import java.util.*;

var sb = new StringBuilder();
sb.append("{\n  \"renames\": [\n");
var fm = currentProgram.getFunctionManager();
var iter = fm.getFunctions(true);
var first = true;
while (iter.hasNext()) {
    var func = iter.next();
    var name = func.getName();
    if (!name.startsWith("FUN_") && !name.startsWith("LAB_")) {
        if (!first) sb.append(",\n");
        first = false;
        sb.append("    {\"address\": \"")
          .append(Long.toHexString(func.getEntryPoint().getOffset()))
          .append("\", \"new_name\": \"")
          .append(name.replace("\"", "\\\""))
          .append("\"}");
    }
}
// Also get non-function labels
var symTable = currentProgram.getSymbolTable();
var symIter = symTable.getSymbolIterator();
while (symIter.hasNext()) {
    var sym = symIter.next();
    var name = sym.getName();
    var addr = sym.getAddress();
    if (!name.startsWith("FUN_") && !name.startsWith("LAB_") && !name.startsWith("DAT_") && !name.startsWith("EXT_") && !name.startsWith("Start") && !name.startsWith("thunk_") && fm.getFunctionContaining(addr) == null) {
        if (!first) sb.append(",\n");
        first = false;
        sb.append("    {\"address\": \"")
          .append(Long.toHexString(addr.getOffset()))
          .append("\", \"new_name\": \"")
          .append(name.replace("\"", "\\\""))
          .append("\"}");
    }
}
sb.append("\n  ]\n}");
println(sb.toString());
```

Capture the output and save to `~/hamsterball-re/analysis/ghidra/renames.json`.

## Restore Renames (run after Ghidra project re-import)

1. Read `~/hamsterball-re/analysis/ghidra/renames.json`
2. Parse the JSON array of renames
3. For each entry, call `mcp_ghidra_mcp_rename_or_label` with the address and new_name
4. Process in batches of ~50 to avoid timeout issues

### Batch Restore Script

```python
import json

with open('/home/evan/hamsterball-re/analysis/ghidra/renames.json') as f:
    data = json.load(f)

renames = data['renames']
print(f"Total renames to apply: {len(renames)}")

# Split into batches of 50
batches = [renames[i:i+50] for i in range(0, len(renames), 50)]
print(f"Batches: {len(batches)}")
```

Then for each batch, call `mcp_ghidra_mcp_batch_create_labels` with the labels array.

## Pitfalls

- The Ghidra project DB is NOT version-controlled. Always export renames to JSON.
- When using `analyzeHeadless` to re-import, all previous renames are LOST unless restored from backup.
- The headless MCP server needs `--program /Hamsterball.exe` (with leading slash) to load a project program.
- Export after every rename batch, or at minimum after each session.

## Quick Reference

| Action | Command |
|--------|---------|
| Export renames | Run inline script, save output to renames.json |
| Restore renames | Read renames.json, batch_create_labels |
| Verify count | mcp_ghidra_mcp_compare_programs_documentation |
| Commit to git | `cd ~/hamsterball-re && git add analysis/ghidra/renames.json && git commit -m "backup renames"` |