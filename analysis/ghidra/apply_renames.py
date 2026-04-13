#!/usr/bin/env python3
"""Apply renames from FUNCTION_MAP.md to GhidraMCP via REST API."""
import json, time, urllib.request, urllib.error, re

BASE_URL = "http://127.0.0.1:8089"

def api_call(endpoint, payload, retries=3):
    url = f"{BASE_URL}/{endpoint}"
    data = json.dumps(payload).encode('utf-8')
    req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'})
    for attempt in range(retries):
        try:
            with urllib.request.urlopen(req, timeout=120) as resp:
                return json.loads(resp.read())
        except Exception as e:
            if attempt < retries - 1:
                print(f"  Retry {attempt+1}: {e}")
                time.sleep(2)
            else:
                return {"error": str(e)}

def parse_function_map(path):
    with open(path) as f:
        content = f.read()
    pattern = r'\|\s*0x([0-9A-Fa-f]+)\s*\|\s*(\w+)\s*\|'
    matches = re.findall(pattern, content)
    # Skip auto-generated names
    skip_prefixes = ('Catch', 'Unwind', 'operator_', 'thunk_', 'entry', 'Start')
    result = []
    seen = set()
    for addr, name in matches:
        padded = addr.upper().zfill(8)
        if padded not in seen and not any(name.startswith(p) for p in skip_prefixes):
            seen.add(padded)
            result.append({"address": padded, "name": name})
    return result

def main():
    renames = parse_function_map('/home/evan/hamsterball-re/docs/FUNCTION_MAP.md')
    print(f"Total renames to apply: {len(renames)}")
    
    batch_size = 50
    total_created = 0
    total_failed = 0
    
    for i in range(0, len(renames), batch_size):
        batch = renames[i:i+batch_size]
        batch_num = i // batch_size
        
        result = api_call("batch_create_labels", {"labels": batch})
        created = result.get("labels_created", 0)
        failed = result.get("labels_failed", 0)
        total_created += created
        total_failed += failed
        
        real_errors = [e for e in result.get("errors", []) if "not snake_case" not in e]
        print(f"Batch {batch_num}: {created} created, {failed} failed, {len(real_errors)} real errors")
        if real_errors:
            for e in real_errors[:3]:
                print(f"  {e}")
        
        time.sleep(0.5)  # Small delay between batches
    
    print(f"\n=== TOTAL: {total_created} created, {total_failed} failed ===")
    
    # Check progress
    print("\nChecking documentation progress...")
    result = api_call("compare_programs_documentation", {})
    if "programs" in result:
        for p in result["programs"]:
            pct = p.get("documentation_percent", 0)
            doc = p.get("documented", 0)
            total = p.get("total_functions", 0)
            print(f"  {pct:.1f}% ({doc}/{total})")

if __name__ == "__main__":
    main()