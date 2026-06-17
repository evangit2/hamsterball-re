#!/usr/bin/env python3
"""Extract interesting strings from any binary."""
import sys, re, subprocess, os

PATTERNS = [
    ("dlls", r"\.dll$", False),
    ("files", r"\.(exe|dll|mesh|world|xml|wav|ogg|png|bmp|jpg|mo3|cfg|txt|json)$", True),
    ("apis", r"direct3d|d3d|dinput|dsound|opengl|vulkan|bass|openal|xinput", True),
    ("c__classes", r"^[A-Z][A-Za-z0-9_]*::[A-Za-z0-9_]+", False),
    ("paths", r"[A-Za-z]:\\|\\|/", True),
]

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <binary>")
        return 1
    path = sys.argv[1]
    try:
        raw = subprocess.check_output(["strings", "-n", "4", path], text=True)
    except Exception as e:
        print("Error running strings:", e)
        return 1
    lines = raw.splitlines()
    for bucket, pat, case in PATTERNS:
        flags = re.IGNORECASE if case else 0
        hits = sorted(set(l for l in lines if re.search(pat, l, flags)))
        print(f"\n=== {bucket.upper()} ({len(hits)}) ===")
        for h in hits[:40]:
            print(h)

if __name__ == "__main__":
    raise SystemExit(main()))
