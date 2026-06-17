#!/usr/bin/env python3
"""Quick health check for GhidraMCP headless server."""
import urllib.request, json, sys

URL = "http://127.0.0.1:8089"

def main():
    try:
        with urllib.request.urlopen(f"{URL}/health", timeout=5) as r:
            data = json.loads(r.read())
        print(json.dumps(data, indent=2))
        if not data.get("program_loaded"):
            print("ERROR: program not loaded", file=sys.stderr)
            return 1
        with urllib.request.urlopen(f"{URL}/compare_programs_documentation", timeout=30) as r:
            cov = json.loads(r.read())
        for p in cov.get("programs", []):
            pct = p.get("documentation_percent", 0)
            doc = p.get("documented", 0)
            tot = p.get("total_functions", 0)
            print(f"Documentation: {pct:.1f}% ({doc}/{tot})")
        return 0
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
