#!/usr/bin/env python3
"""Extract all T: (decal) objects and their diffuse alpha values from all
Hamsterball race + arena main level files. Outputs a .txt listing."""
import struct, os, sys

LEVELS_DIR = "/home/evan/hamsterball-re/originals/installed/extracted/Levels"

# 15 race + 15 arena main level files
RACES = ["Level1","Level2","Level3","Level4","Level5","Level6","Level7",
         "Level8","Level9","Level10","LevelCascade","LevelUp","LevelDark",
         "LevelGlass","LevelImpossible"]
ARENAS = ["Arena-Beginner","Arena-Dizzy","Arena-Expert","Arena-Glass",
          "Arena-Impossible","Arena-Intermediate","Arena-Master","Arena-Neon",
          "Arena-Odd","Arena-Sky","Arena-Toob","Arena-Tower","Arena-Up",
          "Arena-WarmUp","Arena-Wobbly"]

def read_string(fh):
    length = struct.unpack('<i', fh.read(4))[0]
    if length <= 0:
        return ""
    return fh.read(length).rstrip(b'\x00').decode('utf-8', errors='replace')

def read_float(fh):
    return struct.unpack('<f', fh.read(4))[0]

def read_int(fh):
    return struct.unpack('<i', fh.read(4))[0]

def parse_material(fh):
    mat = {
        'ambient': [read_float(fh) for _ in range(4)],
        'diffuse': [read_float(fh) for _ in range(4)],
        'specular': [read_float(fh) for _ in range(4)],
        'emissive': [read_float(fh) for _ in range(4)],
        'power': read_float(fh),
        'has_reflection': read_int(fh),
    }
    has_tex = read_int(fh)
    mat['has_texture'] = has_tex
    mat['texture'] = None
    if has_tex:
        mat['texture'] = read_string(fh)
    return mat

def parse_octree(fh, geoms):
    [read_float(fh) for _ in range(3)]
    [read_float(fh) for _ in range(3)]
    submesh_count = read_int(fh)
    if submesh_count > 0:
        for _ in range(submesh_count):
            parse_octree(fh, geoms)
    else:
        geom_count = read_int(fh)
        for _ in range(geom_count):
            name = read_string(fh)
            material = parse_material(fh)
            strip_count = read_int(fh)
            for _ in range(strip_count):
                read_int(fh)  # tri_count
                read_int(fh)  # vertex_offset
            geoms.append({'name': name, 'material': material})

def parse_file(path):
    with open(path, 'rb') as fh:
        # Section 1: refs
        rpc = read_int(fh)
        for _ in range(rpc):
            read_string(fh)
            [read_float(fh) for _ in range(6)]  # pos + rot
            has_mat = struct.unpack('<I', fh.read(4))[0] & 0xFF
            if has_mat:
                parse_material(fh)
        # Section 2: splines
        sc = read_int(fh)
        for _ in range(sc):
            read_string(fh)
            pc = read_int(fh)
            for _ in range(pc):
                [read_float(fh) for _ in range(3)]
        # Section 3: lights
        lc = read_int(fh)
        for _ in range(lc):
            lt = read_int(fh)
            if lt == 0:
                [read_float(fh) for _ in range(9)]
        # Section 4: colors
        [read_float(fh) for _ in range(6)]
        # Section 5: vertices
        vc = read_int(fh)
        for _ in range(vc):
            [read_float(fh) for _ in range(8)]
        # Section 6: octree
        geoms = []
        parse_octree(fh, geoms)
        return geoms

def fmt_col(c):
    """Format a float color as 0-255 int."""
    return int(round(max(0.0, min(1.0, c)) * 255))

def fmt_alpha(a):
    return f"{a:.2f}"

out = []
out.append("Hamsterball T: (Decal) Object Alpha Values")
out.append("=" * 60)
out.append("Alpha = 4th component (A) of the material's DIFFUSE color.")
out.append("Reported as a 0.0-1.0 float and as 0-255 byte equivalent.")
out.append("")

for cat, files in [("RACES", RACES), ("ARENAS", ARENAS)]:
    out.append("")
    out.append("=" * 60)
    out.append(f"  {cat}")
    out.append("=" * 60)
    for level in files:
        path = os.path.join(LEVELS_DIR, level + ".MESHWORLD")
        if not os.path.exists(path):
            out.append(f"\n[{level}]  (file not found)")
            continue
        geoms = parse_file(path)
        t_geoms = [g for g in geoms if g['name'].startswith('T:')]
        out.append(f"\n[{level}]  T: objects = {len(t_geoms)}")
        if not t_geoms:
            out.append("  (none)")
            continue
        for g in sorted(t_geoms, key=lambda x: x['name']):
            m = g['material']
            d = m['diffuse']
            a = d[3]
            tex = m['texture'] if m['has_texture'] else "(solid)"
            a_byte = fmt_alpha(a)
            out.append(
                f"  {g['name']:32s} "
                f"diffuse RGBA=({fmt_col(d[0])},{fmt_col(d[1])},{fmt_col(d[2])},{fmt_col(a)}) "
                f"alpha={a_byte}  [{tex}]"
            )

outtxt = "\n".join(out) + "\n"
outpath = "/home/evan/hamsterball-re/T_alpha_values.txt"
with open(outpath, 'w') as f:
    f.write(outtxt)
print(outtxt)
print(f"\nWROTE: {outpath}")