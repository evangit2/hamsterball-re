#!/usr/bin/env python3
"""
verify_struct_offsets.py - Automated struct field verification via GhidraMCP.
Edit FUNCTIONS_TO_DECOMPILE and CLAIMED_OFFSETS per target.
"""
import re, json, urllib.request, sys

GHIDRA_MCP_URL = "http://127.0.0.1:8089"

# --- CONFIG: edit per target ---
STRUCT_NAME = "Scene" if len(sys.argv) < 2 else sys.argv[1]
FUNCTIONS_TO_DECOMPILE = [
    (0x419C00, "Scene_Update"),
    (0x419FA0, "Scene_SetCamera"),
    (0x419B30, "Scene_dtor"),
    (0x41A5B0, "Scene_Render"),
    (0x41B2C0, "Level_UpdateAndRender"),
    (0x4564C0, "Ball_AdvancePositionOrCollision"),
    (0x405E00, "Ball_Update"),
    (0x4039E0, "Ball_ctor2"),
]

CLAIMED_OFFSETS = {
    0x000: "vtable",
    0x29D4: "ball_array",
    0x29BC: "camera_orbit_angle",
    0x29C0: "camera_orbit_dist",
}

# --- ENGINE ---

def decompile(addr):
    url = f"{GHIDRA_MCP_URL}/decompile_function?address=0x{addr:06X}"
    try:
        with urllib.request.urlopen(url, timeout=60) as resp:
            data = json.loads(resp.read())
        return data.get("decompiled_code", "") or data.get("code", "")
    except Exception as e:
        return f"/* ERROR: {e} */"

def extract_offsets(c_code):
    offsets = set()
    for m in re.finditer(r'\(int\)(?:param_1|this) \+ (0x[0-9a-fA-F]+)', c_code):
        offsets.add(int(m.group(1), 16))
    for m in re.finditer(r'(?:param_1|this)\[(0x[0-9a-fA-F]+)\]', c_code):
        offsets.add(int(m.group(1), 16) * 4)
    return offsets

def main():
    print(f"# {STRUCT_NAME} Struct Verification Report")
    offset_refs = {}
    for addr, name in FUNCTIONS_TO_DECOMPILE:
        c_code = decompile(addr)
        offsets = extract_offsets(c_code)
        for off in offsets:
            offset_refs.setdefault(off, {})[name] = True
        print(f"- **{name}** (0x{addr:06X}): {len(offsets)} unique offsets")

    print("\n## Claimed Offset Verification")
    print("\n| Offset | Name | Refs | Marker |")
    print("|--------|------|------|--------|")
    for off, name in sorted(CLAIMED_OFFSETS.items()):
        refs = offset_refs.get(off, {})
        count = len(refs)
        marker = "✅" if count >= 2 else ("⚠️" if count == 1 else "❓")
        print(f"| +0x{off:04X} | {name} | {count} | {marker} |")

    unclaimed = {off: refs for off, refs in offset_refs.items() if off not in CLAIMED_OFFSETS}
    print("\n## Unclaimed Offsets (Potential New Fields)")
    for off in sorted(unclaimed.keys()):
        print(f"- +0x{off:04X}: {len(unclaimed[off])} refs — {', '.join(unclaimed[off].keys())}")

if __name__ == "__main__":
    main()
