---
name: ghidra-restore-renames
description: Restore all function renames from FUNCTION_MAP.md back into a fresh Ghidra project after DB loss or re-import
version: 2026-04-13
---

# Ghidra Rename Restore from Git

When the Ghidra project DB is lost/re-imported, recover all renames from `docs/FUNCTION_MAP.md`.

## Source of Truth

`~/hamsterball-re/docs/FUNCTION_MAP.md` — the authoritative list of all renamed functions.
Format: `| 0x004XXXXX | FunctionName | Description |`

## Proven Restore Process (tested Apr 2026)

### Step 1: Start GhidraMCP Server

```bash
export GHIDRA_HOME=/opt/ghidra_12.0.4_PUBLIC
MCP_JAR=~/.config/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP/lib/GhidraMCP-5.2.0.jar
CLASSPATH="$MCP_JAR"
for jar in $GHIDRA_HOME/Ghidra/Framework/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Features/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Processors/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
nohup java -Xmx4g -XX:+UseG1GC -Dghidra.home=$GHIDRA_HOME -Dapplication.name=GhidraMCP \
    -classpath "$CLASSPATH" com.xebyte.headless.GhidraMCPHeadlessServer \
    --port 8089 --bind 127.0.0.1 \
    --project ~/hamsterball-re/analysis/ghidra/HamsterballProject/Hamsterball.gpr \
    --program /Hamsterball.exe > /tmp/ghidra-mcp.log 2>&1 &
sleep 15
curl -s http://127.0.0.1:8089/health
```

### Step 2: Run the Restore Script

```bash
cd ~/hamsterball-re && python3 analysis/ghidra/apply_renames.py
```

The script:
1. Parses FUNCTION_MAP.md for all address+name pairs using regex `\|\s*0x([0-9A-Fa-f]+)\s*\|\s*(\w+)\s*\|`
2. Sends batches of 50 to `http://127.0.0.1:8089/batch_create_labels`
3. Reports progress per batch
4. Shows final documentation percentage

Results from Apr 2026: 840/841 labels created, 0 failed. 40% -> 61% documented.

### Step 3: Verify

```bash
curl -s http://127.0.0.1:8089/compare_programs_documentation
```

## Additional Sources

- `~/hamsterball-re/docs/KEY_FINDINGS.md` — key address references
- `~/hamsterball-re/docs/STRUCTS_AND_TYPES.md` — struct definitions
- `~/hamsterball-re/analysis/ghidra/renames_backup.json` — last backup export

## Pitfalls

- Addresses in FUNCTION_MAP.md use `0x` prefix, but REST API wants raw hex like `00412345`
- Some names may have conflicts (thunks, duplicates) — script continues on error
- BASS/D3D/DInput IAT thunks are auto-named by Ghidra — skip those
- Skip entries where name starts with: `Catch@`, `Unwind@`, `operator_`, `thunk_`, `entry`, `Start`