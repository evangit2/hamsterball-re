# 02 - Ghidra Setup and GhidraMCP Headless Server

## 1. Import the Binary

Use Ghidra's headless analyzer (do this once):

```bash
export GHIDRA_HOME=/opt/ghidra_12.0.4_PUBLIC
mkdir -p ~/hamsterball-re/analysis/ghidra/HamsterballProject
$GHIDRA_HOME/support/analyzeHeadless \
    ~/hamsterball-re/analysis/ghidra/HamsterballProject \
    Hamsterball \
    -import ~/hamsterball-re/originals/installed/extracted/Hamsterball.exe \
    -overwrite
```

**Warning:** `-overwrite` wipes any existing renames. Only do this if restoring from `FUNCTION_MAP.md` afterwards.

## 2. Install / Update GhidraMCP Extension

The project uses GhidraMCP for REST-based decompilation. Extension path:

```
~/.config/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP/lib/GhidraMCP-5.12.0.jar
```

Update procedure:

```bash
cd /tmp && mkdir -p ghidra-mcp-update && cd ghidra-mcp-update
curl -sLO https://github.com/benethington/ghidra-mcp/releases/latest/download/GhidraMCP.zip
curl -sLO https://github.com/benethington/ghidra-mcp/releases/latest/download/bridge_mcp_ghidra.py
unzip -q GhidraMCP.zip -d ~/.config/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP
sudo mkdir -p /opt/ghidra-mcp
sudo cp bridge_mcp_ghidra.py /opt/ghidra-mcp/
```

## 3. Start the Headless Server

Use a Hermes background terminal process (do NOT use `nohup ... &` in foreground):

```bash
export GHIDRA_HOME=/opt/ghidra_12.0.4_PUBLIC
MCP_JAR=/home/evan/.config/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP/lib/GhidraMCP-5.12.0.jar
CLASSPATH="$MCP_JAR"
for jar in $GHIDRA_HOME/Ghidra/Framework/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Features/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done
for jar in $GHIDRA_HOME/Ghidra/Processors/*/lib/*.jar; do CLASSPATH="${CLASSPATH}:${jar}"; done

java -Xmx4g -XX:+UseG1GC \
    -Dghidra.home=$GHIDRA_HOME -Dapplication.name=GhidraMCP \
    -classpath "$CLASSPATH" \
    com.xebyte.headless.GhidraMCPHeadlessServer \
    --port 8089 --bind 127.0.0.1 \
    --project /home/evan/hamsterball-re/analysis/ghidra/HamsterballProject/Hamsterball.gpr \
    --program /Hamsterball.exe \
    > /tmp/ghidra-mcp.log 2>&1
```

**Required flags:**
- `--project` must end in `.gpr`
- `--program` must have leading slash (`/Hamsterball.exe`)

## 4. Verify the Server

```bash
# Health check
curl -s http://127.0.0.1:8089/health

# Expected output:
# {"status":"healthy","version":"5.12.0-headless","program_loaded":true,"program_name":"Hamsterball.exe"}

# Rename coverage
curl -s http://127.0.0.1:8089/compare_programs_documentation

# List functions
curl -s "http://127.0.0.1:8089/list_functions?page=1&limit=5000" | head

# Decompile a known function
curl -s "http://127.0.0.1:8089/decompile_function?address=0x004278E0"
```

## 5. Hermes MCP Bridge (Optional)

If Hermes does not auto-discover `mcp_ghidra_mcp_*` tools, enable the bridge in `~/.hermes/config.yaml`:

```yaml
mcp_servers:
  ghidra-mcp:
    command: python3
    args:
      - /opt/ghidra-mcp/bridge_mcp_ghidra.py
    env:
      GHIDRA_SERVER_URL: http://127.0.0.1:8089/
    timeout: 180
    connect_timeout: 30
```

Then verify with `hermes tools list | grep ghidra`.

## 6. Important Server Quirks

- **Use GET with query params**, not POST with JSON. Direct POST to `/decompile_function` returns errors.
- `/list_functions` returns plain text, not JSON.
- Name lookups (`?name=Foo`) are unreliable; prefer `?address=0x...`.
- `run_script_inline` is broken in headless mode due to OSGi BundleHost issues.

## 7. Stop / Restart

```bash
kill $(pgrep -f GhidraMCPHeadlessServer)
# Wait 2 seconds, then re-run the start command.
```
