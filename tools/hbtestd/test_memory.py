#!/usr/bin/env python3
"""Comprehensive tests for hbtestd memory, addresses, and monitor modules.

Tests run against the test process itself (self-process memory) to verify
all read/write/scan/chain/monitor/freeze operations work correctly.
"""
import os
import struct
import sys
import time
import tempfile

# Add the package to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from hbtestd.memory import MemoryManager, _parse_addr
from hbtestd.addresses import (
    APP, BALL, SCENE, RUMBLE_BOARD, SCENE_OBJECT, GADGET,
    G_APP_PTR, IMAGE_BASE, FUNCTIONS, VTABLES,
    list_known_symbols, get_struct_layout, resolve_symbol,
)
from hbtestd.monitor import AddressMonitor, FreezeManager

PASS = 0
FAIL = 0
ERRORS = []

def check(name: str, condition: bool, detail: str = ""):
    global PASS, FAIL
    if condition:
        PASS += 1
        print(f"  ✓ {name}")
    else:
        FAIL += 1
        ERRORS.append(f"{name}: {detail}")
        print(f"  ✗ {name} — {detail}")

# ---------------------------------------------------------------------------
# Setup: allocate memory in this process we can read/write
# ---------------------------------------------------------------------------
import ctypes
# Allocate a page of memory we can use for testing
libc = ctypes.CDLL("libc.so.6")
libc.mmap.restype = ctypes.c_void_p
libc.mmap.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_long]
libc.munmap.argtypes = [ctypes.c_void_p, ctypes.c_size_t]

PROT_READ = 1
PROT_WRITE = 2
PROT_EXEC = 4
MAP_PRIVATE = 0x02
MAP_ANONYMOUS = 0x20
MAP_32BIT = 0x40  # x86_64 only: map in low 2GB (32-bit addressable)

# Map 64KB in the low 32-bit address space (so pointers fit in u32)
test_mem = libc.mmap(None, 65536, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0)
if not test_mem or test_mem == ctypes.c_void_p(-1).value:
    print("FATAL: mmap failed")
    sys.exit(1)

test_addr = ctypes.cast(test_mem, ctypes.c_void_p).value
assert test_addr < 0x100000000, f"mmap didn't return 32-bit address: 0x{test_addr:X}"
print(f"Test memory at 0x{test_addr:08X}")

# Write some known data to test memory
# u32 value 0xDEADBEEF at offset 0
struct.pack_into("<I", ctypes.cast(test_mem, ctypes.POINTER(ctypes.c_char))[0] if False else (ctypes.c_char * 65536).from_address(test_addr), 0, 0xDEADBEEF)
# Actually use ctypes memset approach
ctypes.memmove(test_addr, struct.pack("<I", 0xDEADBEEF), 4)
ctypes.memmove(test_addr + 4, struct.pack("<I", 0xCAFEBABE), 4)
ctypes.memmove(test_addr + 8, struct.pack("<f", 3.14159), 4)
ctypes.memmove(test_addr + 12, struct.pack("<fff", 1.0, 2.0, 3.0), 12)
ctypes.memmove(test_addr + 24, struct.pack("<q", 0x1234567890ABCDEF), 8)
ctypes.memmove(test_addr + 34, b"Hello World\x00", 12)
ctypes.memmove(test_addr + 48, struct.pack("<d", 2.71828), 8)

# Create a pointer chain structure:
# [test_addr + 64] = pointer to test_addr + 80
# [test_addr + 80] = pointer to test_addr + 96
# [test_addr + 96] = 0x42424242
ptr1 = test_addr + 80
ptr2 = test_addr + 96
ctypes.memmove(test_addr + 64, struct.pack("<I", ptr1), 4)
ctypes.memmove(test_addr + 80, struct.pack("<I", ptr2), 4)
ctypes.memmove(test_addr + 96, struct.pack("<I", 0x42424242), 4)

mm = MemoryManager(os.getpid())

# ===========================================================================
# 1. Basic typed reads
# ===========================================================================
print("\n=== 1. Typed Reads ===")

check("read_u32", mm.read_u32(test_addr) == 0xDEADBEEF, f"got 0x{mm.read_u32(test_addr):08X}")
check("read_u32 offset 4", mm.read_u32(test_addr + 4) == 0xCAFEBABE, f"got 0x{mm.read_u32(test_addr+4):08X}")
check("read_float", abs(mm.read_float(test_addr + 8) - 3.14159) < 0.001, str(mm.read_float(test_addr + 8)))
check("read_vec3", mm.read_vec3(test_addr + 12) == (1.0, 2.0, 3.0), str(mm.read_vec3(test_addr + 12)))
check("read_i64", mm.read_i64(test_addr + 24) == 0x1234567890ABCDEF, f"got 0x{mm.read_i64(test_addr+24):016X}")
check("read_string", mm.read_string(test_addr + 34) == "Hello World", repr(mm.read_string(test_addr + 34)))
check("read_double", abs(mm.read_double(test_addr + 48) - 2.71828) < 0.001, str(mm.read_double(test_addr + 48)))
check("read_u8", mm.read_u8(test_addr) == 0xEF, f"got 0x{mm.read_u8(test_addr):02X}")
check("read_u16", mm.read_u16(test_addr) == 0xBEEF, f"got 0x{mm.read_u16(test_addr):04X}")
check("read_bool false", mm.read_bool(test_addr + 34 + 11) == False)  # null byte after string
check("read_bool true", mm.read_bool(test_addr) == True)  # non-zero

# ===========================================================================
# 2. Typed writes
# ===========================================================================
print("\n=== 2. Typed Writes ===")

# Write u32
mm.write_u32(test_addr + 100, 0xAABBCCDD)
check("write_u32", mm.read_u32(test_addr + 100) == 0xAABBCCDD, f"got 0x{mm.read_u32(test_addr+100):08X}")

# Write float
mm.write_float(test_addr + 104, 99.5)
check("write_float", abs(mm.read_float(test_addr + 104) - 99.5) < 0.001, str(mm.read_float(test_addr + 104)))

# Write vec3
mm.write_vec3(test_addr + 108, 10.0, 20.0, 30.0)
check("write_vec3", mm.read_vec3(test_addr + 108) == (10.0, 20.0, 30.0), str(mm.read_vec3(test_addr + 108)))

# Write i64
mm.write_i64(test_addr + 120, -123456789)
check("write_i64", mm.read_i64(test_addr + 120) == -123456789, str(mm.read_i64(test_addr + 120)))

# Write string
mm.write_string(test_addr + 130, "TestStr")
check("write_string", mm.read_string(test_addr + 130) == "TestStr", repr(mm.read_string(test_addr + 130)))

# Write bool
mm.write_bool(test_addr + 140, True)
check("write_bool true", mm.read_bool(test_addr + 140) == True)
mm.write_bool(test_addr + 140, False)
check("write_bool false", mm.read_bool(test_addr + 140) == False)

# Write via write_typed dispatcher
mm.write_typed(test_addr + 144, 42, "u32")
check("write_typed u32", mm.read_u32(test_addr + 144) == 42, str(mm.read_u32(test_addr + 144)))
mm.write_typed(test_addr + 148, 1.5, "float")
check("write_typed float", abs(mm.read_float(test_addr + 148) - 1.5) < 0.001)

# Write bytes
mm.write_bytes(test_addr + 152, b"\x01\x02\x03\x04\x05\x06")
check("write_bytes", mm.read_bytes(test_addr + 152, 6) == b"\x01\x02\x03\x04\x05\x06")

# ===========================================================================
# 3. read_typed / write_typed dispatcher
# ===========================================================================
print("\n=== 3. Typed Dispatcher ===")

for dtype, val, check_val in [
    ("u8", 200, 200),
    ("i8", -50, -50),
    ("u16", 50000, 50000),
    ("i16", -30000, -30000),
    ("u32", 4000000000, 4000000000),
    ("i32", -2000000000, -2000000000),
    ("u64", 9000000000, 9000000000),
    ("i64", -5000000000, -5000000000),
    ("float", 3.14, 3.14),
    ("double", 2.71828, 2.71828),
    ("bool", True, True),
]:
    offset = 200 + hash(dtype) % 200
    mm.write_typed(test_addr + offset, val, dtype)
    result = mm.read_typed(test_addr + offset, dtype)
    if dtype in ("float", "double"):
        check(f"dispatch {dtype}", abs(result - check_val) < 0.01, f"got {result}")
    else:
        check(f"dispatch {dtype}", result == check_val, f"got {result}")

# vec3 via dispatcher
mm.write_typed(test_addr + 400, [7.0, 8.0, 9.0], "vec3")
v = mm.read_typed(test_addr + 400, "vec3")
check("dispatch vec3 list", v == (7.0, 8.0, 9.0), str(v))
mm.write_typed(test_addr + 400, {"x": 1.0, "y": 2.0, "z": 3.0}, "vec3")
v = mm.read_typed(test_addr + 400, "vec3")
check("dispatch vec3 dict", v == (1.0, 2.0, 3.0), str(v))

# ===========================================================================
# 4. read_struct (multi-field)
# ===========================================================================
print("\n=== 4. read_struct ===")

# Write a fake struct
ctypes.memmove(test_addr + 500, struct.pack("<iff", 42, 1.5, 2.5), 12)
fields = [("id", 0, "u32"), ("x", 4, "float"), ("y", 8, "float")]
s = mm.read_struct(test_addr + 500, fields)
check("read_struct id", s["id"] == 42, str(s))
check("read_struct x", abs(s["x"] - 1.5) < 0.001, str(s))
check("read_struct y", abs(s["y"] - 2.5) < 0.001, str(s))

# ===========================================================================
# 5. Pointer chain resolution
# ===========================================================================
print("\n=== 5. Pointer Chains ===")

# Test resolve_pointer_chain
final = mm.resolve_pointer_chain(test_addr + 64, [0x00, 0x00])
check("pointer_chain final", final == test_addr + 96, f"got 0x{final:08X}, expected 0x{test_addr+96:08X}")
check("pointer_chain value", mm.read_u32(final) == 0x42424242, f"got 0x{mm.read_u32(final):08X}")

# Test read_pointer
ptr = mm.read_pointer(test_addr + 64)
check("read_pointer", ptr == ptr1, f"got 0x{ptr:08X}, expected 0x{ptr1:08X}")

# Test dereference
deref = mm.dereference(test_addr + 64, 0)
check("dereference", deref == ptr1, f"got 0x{deref:08X}")

# ===========================================================================
# 6. Batch operations
# ===========================================================================
print("\n=== 6. Batch Operations ===")

# Write some values
mm.write_u32(test_addr + 600, 111)
mm.write_float(test_addr + 604, 22.5)
mm.write_u16(test_addr + 608, 999)

reads = [
    {"label": "a", "address": test_addr + 600, "type": "u32"},
    {"label": "b", "address": test_addr + 604, "type": "float"},
    {"label": "c", "address": test_addr + 608, "type": "u16"},
    {"label": "bad", "address": 0xFFFFFFFF, "type": "u32"},  # should fail
]
results = mm.read_batch(reads)
check("read_batch count", len(results) == 4, f"got {len(results)}")
check("read_batch a", results[0]["value"] == 111 and results[0]["ok"], str(results[0]))
check("read_batch b", abs(results[1]["value"] - 22.5) < 0.001 and results[1]["ok"], str(results[1]))
check("read_batch c", results[2]["value"] == 999 and results[2]["ok"], str(results[2]))
check("read_batch bad handled", not results[3]["ok"], str(results[3]))

# Write batch
writes = [
    {"address": test_addr + 700, "value": 0xAAAA, "type": "u32"},
    {"address": test_addr + 704, "value": 55.5, "type": "float"},
    {"address": test_addr + 708, "value": [1.0, 2.0, 3.0], "type": "vec3"},
]
wresults = mm.write_batch(writes)
check("write_batch count", len(wresults) == 3, f"got {len(wresults)}")
check("write_batch u32", wresults[0]["ok"] and mm.read_u32(test_addr + 700) == 0xAAAA, str(wresults[0]))
check("write_batch float", wresults[1]["ok"] and abs(mm.read_float(test_addr + 704) - 55.5) < 0.001, str(wresults[1]))
check("write_batch vec3", wresults[2]["ok"] and mm.read_vec3(test_addr + 708) == (1.0, 2.0, 3.0), str(wresults[2]))

# ===========================================================================
# 7. Value scanning
# ===========================================================================
print("\n=== 7. Value Scanning ===")

# Place a known u32 value
scan_target = 0x12345678
mm.write_u32(test_addr + 800, scan_target)
mm.write_u32(test_addr + 804, scan_target)

# Scan for it (limited to our mmaped region to be fast)
results = mm.scan_value(scan_target, "u32", start=test_addr, end=test_addr + 65536, max_results=50)
check("scan_value found", len(results) >= 2, f"found {len(results)} matches")
found_addrs = {r["address"] for r in results}
check("scan_value addr1", (test_addr + 800) in found_addrs, f"expected 0x{test_addr+800:08X} in {[hex(a) for a in found_addrs]}")
check("scan_value addr2", (test_addr + 804) in found_addrs, f"expected 0x{test_addr+804:08X} in {[hex(a) for a in found_addrs]}")

# Scan_next: change one value and re-check
mm.write_u32(test_addr + 800, 0x87654321)
addrs = [r["address"] for r in results]
results2 = mm.scan_next(0x87654321, "u32", prev_results=addrs)
check("scan_next narrowed", len(results2) == 1, f"got {len(results2)} matches")
check("scan_next addr", results2[0]["address"] == test_addr + 800, f"got 0x{results2[0]['address']:08X}")

# Float range scan
mm.write_float(test_addr + 900, 42.5)
mm.write_float(test_addr + 904, 42.8)
results3 = mm.scan_float_range(42.0, 43.0, start=test_addr, end=test_addr + 65536, max_results=50)
check("scan_float_range found", len(results3) >= 2, f"found {len(results3)} matches")

# ===========================================================================
# 8. Hex dump
# ===========================================================================
print("\n=== 8. Hex Dump ===")

dump = mm.dump_hex(test_addr + 800, 16)
check("dump_hex has lines", "0x" in dump["hex"], dump["hex"][:80])
check("dump_hex raw", len(dump["raw_hex"]) == 32, f"got {len(dump['raw_hex'])} chars")
check("dump_hex contains value", "78 56 34 12" in dump["hex"], dump["hex"][:120])

# ===========================================================================
# 9. Address resolution (resolve_address)
# ===========================================================================
print("\n=== 9. Address Resolution ===")

# Absolute hex
addr = mm.resolve_address("0x005341E0")
check("resolve absolute", addr == 0x005341E0, f"got 0x{addr:08X}" if addr else "None")

# Decimal
addr = mm.resolve_address("5436128")
check("resolve decimal", addr == 5436128, f"got {addr}")

# RVA — resolves against main module. In a real Wine process this would
# be Hamsterball.exe. For the Python test process, the binary may not be
# mapped with a name, so None is an acceptable result.
addr = mm.resolve_address("RVA:0x1000")
check("resolve RVA (no crash)", addr is None or addr > 0, f"got {addr}")

# Module — same caveat
addr = mm.resolve_address("module:libc.so.6:0x0")
check("resolve module (no crash)", addr is None or addr > 0, f"got {addr}")

# Pointer chain via spec
addr = mm.resolve_address(f"chain:0x{test_addr + 64:X},0,0")
check("resolve chain spec", addr == test_addr + 96, f"got 0x{addr:08X}" if addr else "None")

# Pointer deref via spec
addr = mm.resolve_address(f"ptr:0x{test_addr + 64:X}")
check("resolve ptr spec", addr == ptr1, f"got 0x{addr:08X}" if addr else "None")

# ===========================================================================
# 10. Addresses module
# ===========================================================================
print("\n=== 10. Addresses Module ===")

symbols = list_known_symbols()
check("symbols has globals", "g_App" in symbols["globals"], str(symbols.get("globals")))
check("symbols has App struct", "target_fps" in symbols["structs"]["App"], str(symbols["structs"].get("App")))
check("symbols has Ball struct", "position" in symbols["structs"]["Ball"], str(symbols["structs"].get("Ball")))
check("symbols has functions", "Ball_Ctor" in symbols["functions"], str(symbols.get("functions")))
check("symbols has vtables", "Ball" in symbols["vtables"], str(symbols.get("vtables")))
check("symbols image_base", symbols["image_base"] == "0x00400000", str(symbols.get("image_base")))

# Struct layout lookup
layout = get_struct_layout("App")
check("get_struct App", layout is not None and "target_fps" in layout, str(layout))
layout = get_struct_layout("Ball")
check("get_struct Ball", layout is not None and "position" in layout, str(layout))
layout = get_struct_layout("NonExistent")
check("get_struct bad", layout is None, str(layout))

# Known offset values
check("App target_fps offset", APP["target_fps"] == 0x16C, f"got 0x{APP['target_fps']:X}")
check("App render_fps offset", APP["render_fps"] == 0x170, f"got 0x{APP['render_fps']:X}")
check("Ball position offset", BALL["position"] == 0x014, f"got 0x{BALL['position']:X}")
check("Ball is_8ball offset", BALL["is_8ball"] == 0x31D, f"got 0x{BALL['is_8ball']:X}")
check("Scene ball_list offset", SCENE["ball_list"] == 0x29D4, f"got 0x{SCENE['ball_list']:X}")

# ===========================================================================
# 11. AddressMonitor
# ===========================================================================
print("\n=== 11. AddressMonitor ===")

# Write a changing value
mm.write_u32(test_addr + 1000, 100)
mon = AddressMonitor(mm, interval=0.05, maxlen=30)
mon.add_watch("counter", test_addr + 1000, "u32")

# Manual sample
s1 = mon.sample_once()
check("monitor sample 1", s1["counter"]["value"] == 100, str(s1))

# Change value and sample again
mm.write_u32(test_addr + 1000, 200)
s2 = mon.sample_once()
check("monitor sample 2", s2["counter"]["value"] == 200, str(s2))

# History
h = mon.get_history("counter", 10)
check("monitor history", len(h) == 2, f"got {len(h)} samples")
check("monitor history val1", h[0]["value"] == 100, str(h[0]))
check("monitor history val2", h[1]["value"] == 200, str(h[1]))

# List watches
watches = mon.list_watches()
check("monitor list", len(watches) == 1 and watches[0]["name"] == "counter", str(watches))

# Remove watch
mon.remove_watch("counter")
check("monitor remove", len(mon.list_watches()) == 0, str(mon.list_watches()))

# Background thread test
mon.add_watch("bg_counter", test_addr + 1000, "u32")
mon.start()
time.sleep(0.3)
mon.stop()
h = mon.get_history("bg_counter", 50)
check("monitor bg thread", len(h) >= 2, f"got {len(h)} samples from bg thread")

# ===========================================================================
# 12. FreezeManager
# ===========================================================================
print("\n=== 12. FreezeManager ===")

# Place initial value
mm.write_u32(test_addr + 1100, 500)
fz = FreezeManager(mm, interval_ms=10)
fz.freeze("test_freeze", test_addr + 1100, 999, "u32")
fz._ensure_running()
time.sleep(0.15)

# Read back — should be 999 even though we try to write 123
mm.write_u32(test_addr + 1100, 123)
time.sleep(0.05)
val = mm.read_u32(test_addr + 1100)
check("freeze holds value", val == 999, f"got {val}")

# List frozen
frozen = fz.list_frozen()
check("freeze list", len(frozen) == 1 and frozen[0]["name"] == "test_freeze", str(frozen))
check("freeze write_count", frozen[0]["write_count"] > 0, f"write_count={frozen[0]['write_count']}")

# Unfreeze
fz.unfreeze("test_freeze")
mm.write_u32(test_addr + 1100, 777)
time.sleep(0.05)
val = mm.read_u32(test_addr + 1100)
check("unfreeze allows change", val == 777, f"got {val}")

# ===========================================================================
# 13. Pattern search
# ===========================================================================
print("\n=== 13. Pattern Search ===")

# Write a known pattern
pattern = b"\xAA\xBB\xCC\xDD\xEE\xFF"
ctypes.memmove(test_addr + 1200, pattern, len(pattern))

# Exact match
results = mm.find_pattern(pattern, start=test_addr, end=test_addr + 65536, max_results=10)
check("find_pattern exact", len(results) >= 1, f"found {len(results)} matches")
check("find_pattern addr", results[0] == test_addr + 1200, f"got 0x{results[0]:08X}")

# Wildcard match
wild_pattern = b"\xAA\xBB" + b"?" + b"\xDD\xEE\xFF"
results = mm.find_pattern(wild_pattern, start=test_addr, end=test_addr + 65536, max_results=10)
check("find_pattern wildcard", len(results) >= 1, f"found {len(results)} matches")

# ===========================================================================
# 14. list_regions
# ===========================================================================
print("\n=== 14. List Regions ===")

regions = mm.list_regions()
check("list_regions count", len(regions) > 0, f"got {len(regions)} regions")
check("list_regions has perms", all("perms" in r for r in regions[:5]), "missing perms")
check("list_regions has path", all("path" in r for r in regions[:5]), "missing path")

# ===========================================================================
# 15. Edge cases
# ===========================================================================
print("\n=== 15. Edge Cases ===")

# Read 0 bytes
check("read 0 bytes", mm.read_bytes(test_addr, 0) == b"", "should return empty")

# Write 0 bytes
check("write 0 bytes", mm.write_bytes(test_addr, b"") == 0, "should return 0")

# Read out of range (should raise)
try:
    mm.read_bytes(0xDEADBEEF, 4)
    check("read bad addr raises", False, "should have raised")
except OSError:
    check("read bad addr raises", True)
except Exception as e:
    check("read bad addr raises", False, f"wrong exception: {e}")

# _parse_addr helper
check("_parse_addr hex", _parse_addr("0xFF") == 255, str(_parse_addr("0xFF")))
check("_parse_addr dec", _parse_addr("255") == 255, str(_parse_addr("255")))
check("_parse_addr backtick", _parse_addr("0x`0050`") == 0x50, str(_parse_addr("0x`0050`")))

# ===========================================================================
# Cleanup
# ===========================================================================
libc.munmap(ctypes.c_void_p(test_mem), 65536)

print(f"\n{'='*60}")
print(f"RESULTS: {PASS} passed, {FAIL} failed, {PASS + FAIL} total")
if ERRORS:
    print("\nFAILURES:")
    for e in ERRORS:
        print(f"  ✗ {e}")
    sys.exit(1)
else:
    print("\n✅ ALL TESTS PASSED")
