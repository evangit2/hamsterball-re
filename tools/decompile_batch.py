#!/usr/bin/env python3
"""
Batch decompiler for Hamsterball RE project.
Decompiles undocumented functions from GhidraMCP, writes analysis files,
commits, and pushes. Designed to be called repeatedly.
"""
import subprocess, json, os, re, sys, time

GHIDRA_MCP = "http://127.0.0.1:8089"
REPO_DIR = os.path.expanduser("~/hamsterball-re")
DECOMP_DIR = os.path.join(REPO_DIR, "analysis/ghidra/decompilations/batch_auto")

def get_all_functions():
    """Get full function list from GhidraMCP."""
    r = subprocess.run(
        ["curl", "-s", f"{GHIDRA_MCP}/list_functions?offset=0&limit=9999"],
        capture_output=True, text=True, timeout=30
    )
    functions = []
    for line in r.stdout.strip().split('\n'):
        if ' at ' in line:
            name, addr = line.rsplit(' at ', 1)
            functions.append((name.strip(), addr.strip()))
    return functions

def get_documented_addrs():
    """Get set of already-documented function addresses."""
    documented = set()
    for root, dirs, files in os.walk(os.path.join(REPO_DIR, "analysis/ghidra/decompilations")):
        for f in files:
            if f.endswith('.c'):
                m = re.search(r'([0-9a-fA-F]{6,8})\.c$', f)
                if m:
                    addr = m.group(1).lower().zfill(8).upper()
                    documented.add(addr)
    return documented

def decompile_function(addr):
    """Decompile a single function via GhidraMCP."""
    r = subprocess.run(
        ["curl", "-s", f"{GHIDRA_MCP}/decompile_function?address=0x{addr}"],
        capture_output=True, text=True, timeout=60
    )
    return r.stdout.strip()

def analyze_function(name, addr, code):
    """Generate a brief analysis/description of the function."""
    lines = code.split('\n')
    
    # Determine category from name
    category = "general"
    name_lower = name.lower()
    if any(x in name_lower for x in ['ball_', 'ball_']):
        category = "ball"
    elif any(x in name_lower for x in ['scene_', 'scene']):
        category = "scene"
    elif any(x in name_lower for x in ['render', 'draw', 'gfx', 'font', 'ui_']):
        category = "rendering"
    elif any(x in name_lower for x in ['sound', 'music', 'audio', 'bass']):
        category = "audio"
    elif any(x in name_lower for x in ['input', 'key', 'joystick', 'mouse']):
        category = "input"
    elif any(x in name_lower for x in ['board', 'rumble', 'arena']):
        category = "gameplay"
    elif any(x in name_lower for x in ['menu', 'screen', 'option', 'credit']):
        category = "ui"
    elif any(x in name_lower for x in ['ctor', '_init', 'create', 'setup', 'load']):
        category = "initialization"
    elif any(x in name_lower for x in ['dtor', 'free', 'cleanup', 'destroy']):
        category = "cleanup"
    elif any(x in name_lower for x in ['level', 'collision', 'mesh', 'object']):
        category = "level"
    elif any(x in name_lower for x in ['vec3', 'matrix', 'math', 'transform']):
        category = "math"
    
    # Extract key information from decompiled code
    calls = re.findall(r'([A-Za-z_][A-Za-z0-9_]*)\s*\(', code)
    calls = [c for c in calls if c not in ('if', 'for', 'while', 'return', 'void', 'int', 'char', 
            'float', 'undefined', 'undefined4', 'undefined1', 'byte', 'short', 'long', 'ulonglong',
            'code', 'this', 'param', 'local', 'puVar', 'uVar', 'iVar', 'fVar', 'ppuVar', 'ppuVar2',
            '__thiscall', '__fastcall', '__cdecl', 'switch', 'case', 'default', 'goto', 'sizeof')]
    unique_calls = list(dict.fromkeys(calls))[:10]  # dedupe, keep order, limit
    
    # Count struct offsets accessed
    offsets = set(re.findall(r'\+ 0x[0-9a-fA-F]+\b', code))
    
    # Identify key patterns
    patterns = []
    if '_free(' in code: patterns.append("frees memory")
    if 'operator_new' in code or '_malloc' in code: patterns.append("allocates memory")
    if 'memset' in code: patterns.append("uses memset")
    if 'strcmp' in code or 'strcpy' in code: patterns.append("string operations")
    if 'vtable' in code.lower() or '(**(code' in code: patterns.append("vtable dispatch")
    if 'ExceptionList' in code: patterns.append("SEH frame")
    if 'Matrix_' in code: patterns.append("matrix math")
    if 'Font_' in code: patterns.append("font/text rendering")
    if 'Sound' in code or 'sound' in code or 'Play' in code: patterns.append("audio")
    if 'Collision' in code or 'collision' in code: patterns.append("collision")
    if 'Render' in code or 'Draw' in code: patterns.append("rendering")
    if 'Input' in code or 'Key' in code: patterns.append("input handling")
    if 'Camera' in code or 'camera' in code: patterns.append("camera")
    if 'Ball' in code: patterns.append("ball operations")
    if 'Scene' in code: patterns.append("scene operations")
    if 'Board' in code: patterns.append("board operations")
    if 'Level' in code: patterns.append("level operations")
    
    # Build description
    sig_match = re.match(r'(void|int|char|float|undefined4|void\s*\*|long|ulonglong)\s+.*?' + re.escape(name) + r'\s*\([^)]*\)', code, re.DOTALL)
    signature = sig_match.group(0) if sig_match else "unknown signature"
    
    desc_parts = [f"Category: {category}"]
    if patterns:
        desc_parts.append(f"Patterns: {', '.join(patterns)}")
    if unique_calls:
        desc_parts.append(f"Calls: {', '.join(unique_calls)}")
    if offsets:
        desc_parts.append(f"Struct offsets accessed: {len(offsets)}")
    desc_parts.append(f"Code size: {len(code)} chars, {len(lines)} lines")
    
    return signature, "\n * ".join(desc_parts)

def write_decomp_file(name, addr, code, analysis_desc, signature):
    """Write a single decompilation file with header."""
    filename = f"{name}_{addr}.c"
    filepath = os.path.join(DECOMP_DIR, filename)
    
    content = f"""/*
 * Function: {name}
 * Address: 0x{addr}
 * Signature: {signature}
 *
 * {analysis_desc}
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch
 */

{code}
"""
    with open(filepath, 'w') as f:
        f.write(content)
    return filepath

def git_commit_push(batch_num, func_names):
    """Git add, commit, and push."""
    commit_msg = f"Add {len(func_names)} function decompilations (auto batch {batch_num}): {', '.join(func_names[:5])}{'...' if len(func_names) > 5 else ''}"
    
    subprocess.run(["git", "add", "analysis/ghidra/decompilations/batch_auto/"], 
                   capture_output=True, cwd=REPO_DIR, timeout=10)
    
    result = subprocess.run(
        ["git", "commit", "-m", commit_msg],
        capture_output=True, text=True, cwd=REPO_DIR, timeout=10
    )
    
    # Push to both remotes
    push_results = []
    for remote in ["origin", "priv"]:
        r = subprocess.run(
            ["git", "push", remote, "master"],
            capture_output=True, text=True, cwd=REPO_DIR, timeout=30
        )
        if r.returncode == 0:
            push_results.append(f"{remote}:ok")
        else:
            push_results.append(f"{remote}:fail")
    
    return commit_msg, push_results

def run_batch(batch_num, batch_size=10):
    """Run one batch of decompilation."""
    os.makedirs(DECOMP_DIR, exist_ok=True)
    
    # Get functions
    all_funcs = get_all_functions()
    documented = get_documented_addrs()
    
    # Filter to undocumented
    undocumented = []
    for name, addr in all_funcs:
        addr_clean = addr.lower().replace('0x', '').zfill(8).upper()
        if addr_clean not in documented:
            undocumented.append((name, addr))
    
    if not undocumented:
        return {"status": "done", "remaining": 0}
    
    batch = undocumented[:batch_size]
    
    # Decompile all
    results = []
    for name, addr in batch:
        code = decompile_function(addr)
        if not code or len(code) < 20 or "404" in code[:50]:
            code = f"// Failed to decompile {name} at 0x{addr}\n// (empty or error response)"
        signature, desc = analyze_function(name, addr, code)
        filepath = write_decomp_file(name, addr, code, desc, signature)
        results.append({
            "name": name,
            "addr": addr,
            "desc": desc,
            "signature": signature,
            "lines": code.count('\n'),
            "chars": len(code)
        })
    
    # Commit and push
    func_names = [r["name"] for r in results]
    commit_msg, push_results = git_commit_push(batch_num, func_names)
    
    # Get commit hash
    hash_r = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"],
        capture_output=True, text=True, cwd=REPO_DIR, timeout=5
    )
    commit_hash = hash_r.stdout.strip()
    
    return {
        "status": "ok",
        "batch_num": batch_num,
        "functions": results,
        "commit": commit_hash,
        "push": push_results,
        "remaining": len(undocumented) - len(batch)
    }

if __name__ == "__main__":
    batch_num = int(sys.argv[1]) if len(sys.argv) > 1 else 1
    result = run_batch(batch_num)
    print(json.dumps(result, indent=2))
