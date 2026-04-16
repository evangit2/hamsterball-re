#!/bin/bash
# Hamsterball Wine API Comparison Test
# Runs both original and reimpl with Wine D3D8 tracing enabled
# Captures API calls, screenshots, and console output
#
# Usage: ./test_api_compare.sh [xvfb_display]
#   e.g.: ./test_api_compare.sh :99
#
# Prerequisites: Xvfb running, wine installed, both EXEs available

set -e

DISPLAY="${1:-:99}"
export DISPLAY

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJ_DIR="$(dirname "$SCRIPT_DIR")"
ORIG_DIR="$PROJ_DIR/originals/installed/extracted"
REIMPL_DIR="$PROJ_DIR/reimpl"
OUT_DIR="$PROJ_DIR/analysis/screenshots/comparison"
mkdir -p "$OUT_DIR"

echo "=== Hamsterball Wine API Comparison Test ==="
echo "Original:  $ORIG_DIR/Hamsterball.exe"
echo "Reimpl:    $REIMPL_DIR/hamsterball.exe"
echo "Output:    $OUT_DIR"
echo ""

# Kill any existing wine processes
wineserver -k 2>/dev/null || true
sleep 1

# --- Test 1: Original Game with D3D8/DInput tracing ---
echo "--- Running Original Game (5 seconds) ---"
cd "$ORIG_DIR"
WINEDEBUG=+d3d8,+dinput,+dsound wine Hamsterball.exe \
    &>"$OUT_DIR/orig_api_trace.log" &
ORIG_PID=$!
sleep 5
kill $ORIG_PID 2>/dev/null || true
wineserver -k 2>/dev/null || true
sleep 1
scrot "$OUT_DIR/orig_screenshot.png" 2>/dev/null || echo "No screenshot"
echo "Original trace saved: $OUT_DIR/orig_api_trace.log"

# --- Test 2: Reimplementation with D3D8/DInput tracing ---
echo ""
echo "--- Running Reimplementation (5 seconds) ---"
cd "$REIMPL_DIR"
WINEDEBUG=+d3d8,+dinput,+dsound wine ./hamsterball.exe \
    &>"$OUT_DIR/reimpl_api_trace.log" &
REIMPL_PID=$!
sleep 5
kill $REIMPL_PID 2>/dev/null || true
wineserver -k 2>/dev/null || true
sleep 1
scrot "$OUT_DIR/reimpl_screenshot.png" 2>/dev/null || echo "No screenshot"
echo "Reimpl trace saved: $OUT_DIR/reimpl_api_trace.log"

# --- Analysis ---
echo ""
echo "=== API Call Summary ==="
echo ""
echo "--- Original D3D8 CreateDevice calls ---"
grep -i "CreateDevice\|SetRenderState\|SetTransform\|SetLight\|Present\|CreateVertexBuffer\|DrawPrimitive" "$OUT_DIR/orig_api_trace.log" 2>/dev/null | head -30 || echo "No D3D8 traces found (game may not have started)"

echo ""
echo "--- Reimpl D3D8 CreateDevice calls ---"
grep -i "CreateDevice\|SetRenderState\|SetTransform\|SetLight\|Present\|CreateVertexBuffer\|DrawPrimitive" "$OUT_DIR/reimpl_api_trace.log" 2>/dev/null | head -30 || echo "No D3D8 traces found"

echo ""
echo "--- Comparison: SetRenderState calls ---"
echo "Original:"
grep -c "SetRenderState" "$OUT_DIR/orig_api_trace.log" 2>/dev/null || echo "0"
echo "Reimpl:"
grep -c "SetRenderState" "$OUT_DIR/reimpl_api_trace.log" 2>/dev/null || echo "0"

echo ""
echo "=== Test Complete ==="
echo "Compare the .log files for detailed API call sequences."