#!/usr/bin/env python3
"""Render Hamsterball MESHWORLD levels to PNG/GIF via Pillow.
Top-down orthographic + isometric views + rotating camera GIF.
"""
import struct, sys, os, math, time
import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, '/home/evan/.hermes/skills/reverse-engineering/hamsterball-meshworld/scripts')
from meshworld_parser import MeshWorldFile, read_int, read_float

# ---------------------------------------------------------------- geometry
def geom_triangles(mw, geom):
    """Yield triangles for a geom using strip semantics (N+2 verts per strip)."""
    verts = mw.vertices
    for strip in geom['strips']:
        n = strip['triangles']
        off = strip['vertex_offset']
        if off + n + 2 > len(verts) or n < 1:
            continue
        for i in range(n):
            v0 = verts[off + i]['pos']
            v1 = verts[off + i + 1]['pos']
            v2 = verts[off + i + 2]['pos']
            yield (v0, v1, v2)

def collect_mesh(mw):
    """Collect all triangles with per-triangle color."""
    tris = []  # (color, [3 verts])
    def walk(node):
        if node['children']:
            for c in node['children']:
                walk(c)
        else:
            for geom in node['geoms']:
                color = PREFIX_COLORS.get(geom['name'][:2] if len(geom['name']) >= 2 else '',
                                          PREFIX_COLORS[''])
                if geom['name'].startswith('N:GLASS'):
                    color = PREFIX_COLORS['N:GLASS']
                for t in geom_triangles(mw, geom):
                    tris.append((color, t))
    walk(mw.octree)
    return tris

# prefix -> RGBA fill
PREFIX_COLORS = {
    '':      (170, 175, 185, 255),   # unnamed solid
    'S:':    (120, 150, 210, 255),   # standard
    'N:':    (235, 90, 90, 255),     # interactive
    'N:GLASS':(190, 90, 230, 255),   # glass
    'E:':    (245, 210, 70, 255),    # event trigger (invisible in game!)
    'T:':    (90, 210, 235, 255),    # decal
    'O:':    (120, 220, 150, 255),   # translucent
    'AT:':   (255, 140, 200, 255),   # animated texture
}

def project(pts, yaw, pitch, dist=1.0):
    """Rotate + weak-perspective project. pts: Nx3 (x,y,z). y up.
    Returns Nx2 screen coords + Nx1 camera-z (for painter sort)."""
    cy, sy = math.cos(yaw), math.sin(yaw)
    cx, sx = math.cos(pitch), math.sin(pitch)
    Ry = np.array([[cy, 0, sy], [0, 1, 0], [-sy, 0, cy]], dtype=np.float64)
    Rx = np.array([[1, 0, 0], [0, cx, -sx], [0, sx, cx]], dtype=np.float64)
    M = Rx @ Ry
    cam = pts @ M.T
    # weak perspective: screen = cam.x * f / (dist - cam.z), screen_y = -cam.y * f / (dist - cam.z)
    f = dist
    depth = (dist - cam[:, 2]).clip(0.5, None)
    sx2 = cam[:, 0] * f / depth
    sy2 = -cam[:, 1] * f / depth
    return sx2, sy2, cam[:, 2]

def render_level(mw, yaw, pitch, size=1200, pad=40, bg=(24, 26, 34)):
    t0 = time.time()
    tris = collect_mesh(mw)
    if not tris:
        return None
    # gather all verts for bounds
    allv = np.array([v for _, t in tris for v in t], dtype=np.float64)  # Nx3
    cx_, cy_, cz_ = allv.mean(axis=0)
    centered = allv - np.array([cx_, cy_, cz_])
    maxr = float(np.abs(centered).max())
    scale = (size / 2 - pad) / max(maxr, 1e-6)

    # project once per vertex, cache by index? simpler: project per-tri
    img = Image.new('RGB', (size, size), bg)
    dr = ImageDraw.Draw(img)
    # sort triangles by camera depth (painter)
    items = []
    for color, t in tris:
        pts = np.array(t, dtype=np.float64) - np.array([cx_, cy_, cz_])
        sx2, sy2, depth = project(pts, yaw, pitch)
        # average depth of the 3 verts
        items.append((float(depth.mean()), color, sx2, sy2))
    items.sort(key=lambda it: -it[0])  # far first
    n = 0
    for _, color, sx2, sy2 in items:
        x = sx2 * scale + size / 2
        y = sy2 * scale + size / 2
        xy = list(zip(x.tolist(), y.tolist()))
        dr.polygon(xy, fill=color)
        n += 1
    print(f"  rendered {n} tris in {time.time()-t0:.1f}s", file=sys.stderr)
    return img

if __name__ == '__main__':
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('file')
    ap.add_argument('--out', required=True)
    ap.add_argument('--yaw', type=float, default=0.0)
    ap.add_argument('--pitch', type=float, default=0.0)  # 0 = top-down
    ap.add_argument('--size', type=int, default=1200)
    a = ap.parse_args()
    mw = MeshWorldFile.parse(a.file)
    img = render_level(mw, a.yaw, a.pitch, size=a.size)
    if img:
        img.save(a.out)
        print(f"saved {a.out}")
    else:
        print("no geometry")
