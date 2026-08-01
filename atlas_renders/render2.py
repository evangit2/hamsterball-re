#!/usr/bin/env python3
"""MESHWORLD textured renderer — software z-buffer rasterizer.
Loads real game textures (bmp/png/tga fallback, case-insensitive),
perspective-correct UV mapping, lambert shading from S5 normals.
"""
import struct, sys, os, math, time
import numpy as np
from PIL import Image

sys.path.insert(0, '/home/evan/.hermes/skills/reverse-engineering/hamsterball-meshworld/scripts')
from meshworld_parser import MeshWorldFile

TEXDIR = '/home/evan/hamsterball-re/originals/installed/extracted/Textures'
_tex_cache = {}
_dir_index = None

def _index_dir():
    global _dir_index
    if _dir_index is None:
        _dir_index = {}
        try:
            for f in os.listdir(TEXDIR):
                _dir_index[f.lower()] = f
        except OSError:
            pass
    return _dir_index

def load_tex(name):
    """Load texture by base name (with or without ext). Returns float32 RGBA (H,W,4) or None."""
    if not name:
        return None
    if name in _tex_cache:
        return _tex_cache[name]
    idx = _index_dir()
    base = name.rsplit('.', 1)[0] if '.' in name else name
    arr = None
    for ext in ('.bmp', '.png', '.tga'):
        fn = idx.get((base + ext).lower())
        if fn is None:
            continue
        try:
            im = Image.open(os.path.join(TEXDIR, fn))
            arr = np.array(im.convert('RGBA')).astype(np.float32) / 255.0
            break
        except Exception:
            arr = None
    _tex_cache[name] = arr
    return arr

def checker_tex(diffuse, size=32):
    """Procedural checker fallback in material diffuse color (signals missing tex)."""
    a = np.zeros((size, size, 4), np.float32)
    for y in range(size):
        for x in range(size):
            c = 0.75 if ((x // 4 + y // 4) % 2 == 0) else 0.35
            a[y, x] = [diffuse[0] * c, diffuse[1] * c, diffuse[2] * c, 1.0]
    return a

def geom_triangles(mw, geom):
    verts = mw.vertices
    for strip in geom['strips']:
        n = strip['triangles']
        off = strip['vertex_offset']
        if off + n + 2 > len(verts) or n < 1:
            continue
        for i in range(n):
            yield (verts[off + i], verts[off + i + 1], verts[off + i + 2])

# fixed light dir (world space, normalized)
_L = np.array([0.45, 0.85, 0.55], np.float64)
_L /= np.linalg.norm(_L)
_AMBIENT = 0.42

class Raster:
    def __init__(self, W, H, bg=(16, 18, 24)):
        self.W, self.H = W, H
        self.bg = np.array(bg, np.float64) / 255.0
        self.img = np.full((H, W, 3), self.bg, np.float32)
        self.zbuf = np.full((H, W), np.inf, np.float32)
        self.cx, self.cy = W / 2.0, H / 2.0

    def draw(self, tri):
        """tri: dict with arrays X,Y (screen), D (depth), U,V, R,G,B (shaded), tex or None."""
        X, Y, D = tri['X'], tri['Y'], tri['D']
        # backface/cull-free: normalize winding to CCW in screen space
        area2 = (X[1] - X[0]) * (Y[2] - Y[0]) - (X[2] - X[0]) * (Y[1] - Y[0])
        if abs(area2) < 1e-9:
            return
        if area2 < 0:
            idx = [0, 2, 1]
            X, Y, D = X[idx], Y[idx], D[idx]
            for k in ('U', 'V', 'R', 'G', 'B'):
                tri[k] = tri[k][idx]
            area2 = -area2
        xmin = max(0, int(np.floor(X.min())))
        xmax = min(self.W - 1, int(np.ceil(X.max())))
        ymin = max(0, int(np.floor(Y.min())))
        ymax = min(self.H - 1, int(np.ceil(Y.max())))
        if xmax < xmin or ymax < ymin:
            return
        xs, ys = np.meshgrid(np.arange(xmin, xmax + 1, dtype=np.float32),
                             np.arange(ymin, ymax + 1, dtype=np.float32))
        # edge functions
        e0 = (X[1] - X[0]) * (ys - Y[0]) - (Y[1] - Y[0]) * (xs - X[0])
        e1 = (X[2] - X[1]) * (ys - Y[1]) - (Y[2] - Y[1]) * (xs - X[1])
        e2 = (X[0] - X[2]) * (ys - Y[2]) - (Y[0] - Y[2]) * (xs - X[2])
        w0 = e0 / area2
        w1 = e1 / area2
        w2 = e2 / area2
        inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
        if not inside.any():
            return
        # perspective-correct interpolation
        iw = 1.0 / np.maximum(D, 1e-6)
        iw_pix = w0 * iw[0] + w1 * iw[1] + w2 * iw[2]
        depth = 1.0 / np.maximum(iw_pix, 1e-9)
        zb = self.zbuf[ymin:ymax + 1, xmin:xmax + 1]
        valid = inside & (depth < zb)
        if not valid.any():
            return
        def lerp(a):
            return (w0 * a[0] * iw[0] + w1 * a[1] * iw[1] + w2 * a[2] * iw[2]) / iw_pix
        # shade = per-vertex shaded color (gouraud)
        shade = lerp(tri['R'] if False else tri['shade']) if False else None
        # use per-vertex shaded RGB channels directly
        R = lerp(tri['R']); G = lerp(tri['G']); B = lerp(tri['B'])
        tex = tri['tex']
        if tex is None:
            rgb = np.stack([R, G, B], axis=-1)
        else:
            TH, TW = tex.shape[0], tex.shape[1]
            U = lerp(tri['U']); V = lerp(tri['V'])
            tu = np.clip(U * (TW - 1), 0, TW - 1).astype(np.int32)
            tv = np.clip(V * (TH - 1), 0, TH - 1).astype(np.int32)
            tr = tex[tv, tu]  # (H,W,4)
            alpha = tr[..., 3:4]
            rgb = tr[..., :3] * np.stack([R, G, B], axis=-1)
            # alpha blend against background
            rgb = rgb * alpha + self.bg * (1.0 - alpha)
        # write
        yy, xx = np.nonzero(valid)
        if len(yy) == 0:
            return
        self.img[ymin + yy, xmin + xx] = rgb[valid]
        self.zbuf[ymin + yy, xmin + xx] = depth[valid]

class Scene:
    def __init__(self, mw, size, pad=40, fov_deg=42, cam_dist_mult=2.6):
        self.mw = mw
        self.size = size
        self.pad = pad
        allv = np.array([v['pos'] for v in mw.vertices], np.float64)
        self.center = allv.mean(axis=0) if len(allv) else np.zeros(3)
        self.maxr = float(np.abs(allv - self.center).max()) if len(allv) else 100.0
        self.D = cam_dist_mult * max(self.maxr, 1e-6)
        self.f = (size / 2 - pad) / math.tan(math.radians(fov_deg / 2))
        # bg color from S4
        bg = mw.background_color
        self.bg = (int(bg[0] * 255), int(bg[1] * 255), int(bg[2] * 255)) if mw.background_color else (16, 18, 24)

    def project(self, pts, yaw, pitch):
        """pts: Nx3 world. Returns (X, Y, D) screen coords + depth."""
        cy, sy = math.cos(yaw), math.sin(yaw)
        cx, sx = math.cos(pitch), math.sin(pitch)
        Ry = np.array([[cy, 0, sy], [0, 1, 0], [-sy, 0, cy]], np.float64)
        Rx = np.array([[1, 0, 0], [0, cx, -sx], [0, sx, cx]], np.float64)
        M = Rx @ Ry
        q = (pts - self.center) @ M.T
        qz = q[:, 2] + self.D
        qz = np.maximum(qz, 0.5)
        X = self.f * q[:, 0] / qz + self.cx0
        Y = -self.f * q[:, 1] / qz + self.cy0
        return X, Y, qz

    def render(self, yaw, pitch, with_tex=True):
        t0 = time.time()
        W = H = self.size
        rast = Raster(W, H, bg=self.bg)
        self.cx0, self.cy0 = W / 2.0, H / 2.0
        cy, sy = math.cos(yaw), math.sin(yaw)
        cx, sx = math.cos(pitch), math.sin(pitch)
        Ry = np.array([[cy, 0, sy], [0, 1, 0], [-sy, 0, cy]], np.float64)
        Rx = np.array([[1, 0, 0], [0, cx, -sx], [0, sx, cx]], np.float64)
        M = Rx @ Ry
        n = 0
        for tri in self._iter_tris(M, with_tex):
            rast.draw(tri)
            n += 1
        img = Image.fromarray((np.clip(rast.img, 0, 1) * 255).astype(np.uint8), 'RGB')
        print(f'  {n} tris in {time.time()-t0:.1f}s', file=sys.stderr)
        return img

    def _iter_tris(self, M, with_tex=True):
        mw = self.mw
        verts = mw.vertices
        # batch all verts once
        P = np.array([v['pos'] for v in verts], np.float64)
        N = np.array([v['normal'] for v in verts], np.float64)
        Q = (P - self.center) @ M.T
        qz = np.maximum(Q[:, 2] + self.D, 0.5)
        X = self.f * Q[:, 0] / qz + self.cx0
        Y = -self.f * Q[:, 1] / qz + self.cy0
        # lambert per vertex
        ndl = np.clip(N @ _L, 0, 1)
        shade = _AMBIENT + (1.0 - _AMBIENT) * ndl
        def walk(node):
            if node['children']:
                for c in node['children']:
                    yield from walk(c)
                return
            for geom in node['geoms']:
                mat = geom['material']
                tex = None
                if with_tex and mat:
                    tex = load_tex(mat['texture'])
                    if mat['texture'] and tex is None:
                        tex = checker_tex(mat['diffuse'])
                dif = np.array(mat['diffuse'][:3] if mat else [0.8, 0.8, 0.8], np.float64)
                for strip in geom['strips']:
                    tn = strip['triangles']
                    off = strip['vertex_offset']
                    if off + tn + 2 > len(verts) or tn < 1:
                        continue
                    for i in range(tn):
                        i0, i1, i2 = off + i, off + i + 1, off + i + 2
                        tri = {
                            'X': np.array([X[i0], X[i1], X[i2]], np.float64),
                            'Y': np.array([Y[i0], Y[i1], Y[i2]], np.float64),
                            'D': np.array([qz[i0], qz[i1], qz[i2]], np.float64),
                            'U': np.array([verts[i0]['uv'][0], verts[i1]['uv'][0], verts[i2]['uv'][0]], np.float64),
                            'V': np.array([verts[i0]['uv'][1], verts[i1]['uv'][1], verts[i2]['uv'][1]], np.float64),
                            'R': np.array([shade[i0] * dif[0], shade[i1] * dif[0], shade[i2] * dif[0]], np.float64),
                            'G': np.array([shade[i0] * dif[1], shade[i1] * dif[1], shade[i2] * dif[1]], np.float64),
                            'B': np.array([shade[i0] * dif[2], shade[i1] * dif[2], shade[i2] * dif[2]], np.float64),
                            'tex': tex,
                        }
                        yield tri
        yield from walk(mw.octree)

def render_level(mw, yaw, pitch, size=760, pad=40):
    sc = Scene(mw, size, pad)
    return sc.render(yaw, pitch)

if __name__ == '__main__':
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument('file')
    ap.add_argument('--out', required=True)
    ap.add_argument('--yaw', type=float, default=0.0)
    ap.add_argument('--pitch', type=float, default=0.0)
    ap.add_argument('--size', type=int, default=760)
    a = ap.parse_args()
    mw = MeshWorldFile.parse(a.file)
    img = render_level(mw, a.yaw, a.pitch, size=a.size)
    img.save(a.out)
    print('saved', a.out)
