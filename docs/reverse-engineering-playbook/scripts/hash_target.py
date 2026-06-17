#!/usr/bin/env python3
"""Record integrity hashes for any target file or directory."""
import hashlib, json, os, sys

def hash_file(path):
    with open(path, "rb") as f:
        data = f.read()
    return {
        "md5": hashlib.md5(data).hexdigest(),
        "sha256": hashlib.sha256(data).hexdigest(),
        "size": len(data),
    }

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <file_or_dir> [output.json]")
        return 1
    target = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else "hashes.json"
    result = {}
    if os.path.isfile(target):
        result[os.path.basename(target)] = hash_file(target)
    elif os.path.isdir(target):
        for root, _, files in os.walk(target):
            for f in files:
                p = os.path.join(root, f)
                result[os.path.relpath(p, target)] = hash_file(p)
    else:
        print("Path not found:", target)
        return 1
    with open(out, "w") as f:
        json.dump(result, f, indent=2)
    print(f"Wrote {len(result)} entries to {out}")

if __name__ == "__main__":
    import json
    raise SystemExit(main()))
