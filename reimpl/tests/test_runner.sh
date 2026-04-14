#!/bin/bash
# test_runner.sh - Run all tests, report results
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${1:-$SCRIPT_DIR/../build}"
ASSETS_DIR="${2:-$SCRIPT_DIR/../../originals/installed/extracted}"

PASS=0; FAIL=0; TOTAL=0; SKIP=0

run_test() {
    local name="$1" cmd="$2" timeout_s="${3:-30}"
    TOTAL=$((TOTAL+1))
    printf "  TEST %-50s " "$name"
    if output=$(timeout "$timeout_s" $cmd 2>&1); then
        PASS=$((PASS+1))
        echo "PASS"
    else
        local rc=$?
        if [ $rc -eq 124 ]; then
            SKIP=$((SKIP+1))
            echo "SKIP (timeout)"
        else
            FAIL=$((FAIL+1))
            echo "FAIL (exit $rc)"
            echo "$output" | head -20 | sed 's/^/    /'
        fi
    fi
}

echo "========================================"
echo "  Hamsterball Reimpl Test Suite"
echo "========================================"
echo "  Build: $BUILD_DIR"
echo "  Assets: $ASSETS_DIR"
echo ""

# Unit tests (no display needed)
echo "--- Unit Tests ---"
for t in "$BUILD_DIR"/test_*; do
    [ -x "$t" ] || continue
    # Skip integration tests
    case "$(basename $t)" in itest_*) continue ;; esac
    run_test "$(basename $t)" "$t"
done

# Integration tests (need X display)
echo ""
echo "--- Integration Tests ---"
export DISPLAY=${DISPLAY:-:99}
for t in "$BUILD_DIR"/itest_*; do
    [ -x "$t" ] || continue
    run_test "$(basename $t)" "timeout 10 $t" 15
done

echo ""
echo "========================================"
echo "  Results: $PASS/$TOTAL passed, $FAIL failed, $SKIP skipped"
echo "========================================"
[ $FAIL -eq 0 ]