---
name: ghidra-mcp-headless
description: Start GhidraMCP headless server with the Hamsterball.exe project — always do this before any RE work
version: 2026-04-13
---

# GhidraMCP Headless Server Startup

## One-Line Start

```bash
export GHIDRA_HOME=/opt/ghidra_12.0.4_PUBLIC
MCP_JAR=/home/evan/.config/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP/lib/GhidraMCP-5.2.0.jar
CLASSPATH="$MCP_JAR"
for jar in $GHIDRA_HOME/Ghidra/Framework/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Features/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Processors/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done

nohup java -Xmx4g -XX:+UseG1GC \
    -Dghidra.home=$GHIDRA_HOME -Dapplication.name=GhidraMCP \
    -classpath "$CLASSPATH" \
    com.xebyte.headless.GhidraMCPHeadlessServer \
    --port 8089 --bind 127.0.0.1 \
    --project /home/evan/hamsterball-re/analysis/ghidra/HamsterballProject/Hamsterball.gpr \
    --program /Hamsterball.exe \
    > /tmp/ghidra-mcp.log 2>&1 &

echo "Started PID $!"
sleep 15
curl -s http://127.0.0.1:8089/health
```

## Startup Checklist

Before doing ANY RE work:

1. **Verify server is running**: `curl -s http://127.0.0.1:8089/health`
2. **Check program loaded**: `{"status":"healthy","version":"5.2.0-headless","program_loaded":true,"program_name":"Hamsterball.exe"}`
3. **Verify rename count**: `mcp_ghidra_mcp_compare_programs_documentation` — should be ~75% if restored
4. **If 40%**: project was re-imported, restore from `renames.json` first

## Startup Flags Reference

| Flag | Required | Notes |
|------|----------|-------|
| `--port 8089` | Yes | Default port |
| `--bind 127.0.0.1` | Yes | Localhost only |
| `--project /path/Hamsterball.gpr` | Yes | Must end in `.gpr` |
| `--program /Hamsterball.exe` | Yes | Leading slash required |
| `-Xmx4g` | Recommended | Min 4GB heap |
| `-Dghidra.home` | Yes | Ghidra installation path |

## If Program Not Loaded

Error: `program_loaded: false` or `Error loading program`:

1. Check log: `tail -20 /tmp/ghidra-mcp.log`
2. If "No .gpr file found": wrong project path, must end in `.gpr`
3. If "Absolute path must begin with '/'": missing leading slash on `--program`
4. If "Import requires GUI mode": binary file path wrong, use `--file` for direct binary load

## If Import Needed (Fresh Binary)

```bash
# Use analyzeHeadless to import first
export GHIDRA_HOME=/opt/ghidra_12.0.4_PUBLIC
$GHIDRA_HOME/support/analyzeHeadless \
    ~/hamsterball-re/analysis/ghidra/HamsterballProject \
    Hamsterball \
    -import ~/hamsterball-re/originals/installed/extracted/Hamsterball.exe \
    -overwrite \
    2>&1 | tail -10
# Then start MCP server with --program flag
# IMPORTANT: After re-import, ALL previous renames are LOST.
# Run ghidra-restore-renames skill to restore from FUNCTION_MAP.md
```

## Server Maintenance

- **Stop server**: `kill $(pgrep -f GhidraMCPHeadlessServer)`
- **Check logs**: `tail -f /tmp/ghidra-mcp.log`
- **Restart if hung**: kill, wait 2s, restart

## MCP Tool Quick Reference

| Task | Tool |
|------|------|
| List functions | `mcp_ghidra_mcp_list_functions_enhanced` |
| Rename function | `mcp_ghidra_mcp_rename_function_by_address` |
| Batch rename | `mcp_ghidra_mcp_batch_rename_function_components` |
| Search functions | `mcp_ghidra_mcp_search_functions_enhanced` |
| Get function info | `mcp_ghidra_mcp_analyze_function_complete` |
| Decompile | `mcp_ghidra_mcp_decompile_function` |
| Set comment | `mcp_ghidra_mcp_set_decompiler_comment` |
| Batch comments | `mcp_ghidra_mcp_batch_set_comments` |
| Get xrefs | `mcp_ghidra_mcp_get_bulk_xrefs` |
| Check progress | `mcp_ghidra_mcp_compare_programs_documentation` |

## Key Gotchas

1. **NEVER start a session without checking `program_loaded: true`**
2. **NEVER assume renames survived a project re-import**
3. The project path must end in `.gpr` (e.g., `Hamsterball.gpr` not `HamsterballProject`)
4. Program name must have leading slash: `/Hamsterball.exe` not `Hamsterball.exe`
5. Import via `analyzeHeadless` before starting MCP server for a new binary