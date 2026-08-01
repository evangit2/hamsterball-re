#!/usr/bin/env python3
"""Overlay S1 ref point markers on top-down level renders."""
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render import project
sys.path.insert(0, '/home/evan/.hermes/skills/reverse-engineering/hamsterball-meshworld/scripts')
from meshworld_parser import MeshWorldFile
from PIL import Image, ImageDraw
import numpy as np

LVLDIR = '/home/evan/hamsterball-re/originals/installed/extracted/Levels'
OUT = '/home/evan/hamsterball-re/atlas_renders'

def classify(name):
    n = name.upper()
    if n.startswith('START'): return 'start'
    if n.startswith('SAFESPOT') or n.startswith('SAFEPOS'): return 'safe'
    if n.startswith('FLAG'): return 'flag'
    if n.startswith('BADBALL') or n == 'BIGBADBALL': return 'badball'
    if n.startswith('SECRET'): return 'secret'
    if n in ('MOUSETRAP','MOUSETRAP '): return 'trap'
    if n.startswith('CAMERA'): return 'cam'
    return None

STYLE = {
    'start':  ((0, 255, 90),  8),
    'safe':   ((80, 160, 255), 4),
    'flag':   ((255, 255, 255), 3),
    'badball':((255, 60, 60), 5),
    'secret': ((255, 220, 40), 5),
    'trap':   ((255, 120, 0), 5),
    'cam':    ((200, 200, 200), 3),
}

def overlay(name, size=760):
    p = os.path.join(LVLDIR, name + '.MESHWORLD')
    if not os.path.exists(p):
        return None
    mw = MeshWorldFile.parse(p)
    img = Image.open(os.path.join(OUT, 'png', name + '.png')).convert('RGB')
    dr = ImageDraw.Draw(img)
    if not mw.ref_points:
        return img
    allv = np.array([v['pos'] for v in mw.vertices], dtype=np.float64)
    if len(allv) == 0:
        return img
    c = allv.mean(axis=0)
    centered = allv - c
    maxr = float(np.abs(centered).max())
    scale = (size / 2 - 40) / max(maxr, 1e-6)
    for rp in mw.ref_points:
        cls = classify(rp['name'])
        if not cls:
            continue
        pt = np.array([rp['pos']], dtype=np.float64) - c
        sx, sy, _ = project(pt, 0.0, 0.0)
        x = float(sx[0]) * scale + size / 2
        y = float(sy[0]) * scale + size / 2
        color, r = STYLE[cls]
        if cls == 'start':
            dr.ellipse([x-r, y-r, x+r, y+r], outline=color, width=2)
            dr.ellipse([x-2, y-2, x+2, y+2], fill=color)
        elif cls == 'flag':
            dr.polygon([(x, y-r), (x+r*0.7, y), (x, y+r), (x-r*0.7, y)], outline=color, width=1)
        else:
            dr.ellipse([x-r, y-r, x+r, y+r], fill=color)
    return img

def main():
    names = ['Level1','Level2','Level3','Level4','Level5','Level6','Level7','Level8',
             'Level9','Level10','LevelCascade','LevelUp','LevelDark','LevelGlass','LevelImpossible',
             'Arena-Beginner','Arena-Intermediate','Arena-WarmUp','Arena-Dizzy','Arena-Toob',
             'Arena-Tower','Arena-Expert','Arena-Neon','Arena-Sky','Arena-Wobbly','Arena-Odd',
             'Arena-Up','Arena-Glass','Arena-Master','Arena-Impossible']
    count = 0
    for n in names:
        img = overlay(n)
        if img:
            img.save(os.path.join(OUT, 'png', n + '_map.png'))
            count += 1
    print(f'overlaid {count} maps')

if __name__ == '__main__':
    main()
