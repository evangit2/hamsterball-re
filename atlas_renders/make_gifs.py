#!/usr/bin/env python3
"""Orbiting camera GIFs for the most complex levels + a 2-panel showcase."""
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render import render_level
sys.path.insert(0, '/home/evan/.hermes/skills/reverse-engineering/hamsterball-meshworld/scripts')
from meshworld_parser import MeshWorldFile
from PIL import Image, ImageDraw

LVLDIR = '/home/evan/hamsterball-re/originals/installed/extracted/Levels'
OUT = '/home/evan/hamsterball-re/atlas_renders'

def orbit_gif(name, pitch_deg=28, size=560, frames=48, dur=80):
    p = os.path.join(LVLDIR, name + '.MESHWORLD')
    if not os.path.exists(p):
        print('missing', p); return None
    mw = MeshWorldFile.parse(p)
    frames_imgs = []
    for i in range(frames):
        yaw = math.radians(i * 360 / frames)
        img = render_level(mw, yaw, pitch=math.radians(pitch_deg), size=size)
        frames_imgs.append(img)
    path = os.path.join(OUT, f'{name}_orbit.gif')
    frames_imgs[0].save(path, save_all=True, append_images=frames_imgs[1:],
                        duration=dur, loop=0, optimize=False)
    print(f'{name}: {frames} frames -> {path}')
    return path

def showcase():
    # 2-panel: map (top) + iso (bottom) per level, text labeled
    names = ['Level3', 'LevelDark', 'LevelImpossible', 'LevelUp']
    for n in names:
        top = Image.open(os.path.join(OUT, 'png', n + '_map.png')).convert('RGB')
        iso = Image.open(os.path.join(OUT, 'png', n + '_iso.png')).convert('RGB')
        top = top.resize((620, 620))
        iso = iso.resize((620, 620))
        panel = Image.new('RGB', (640, 1260), (12, 13, 18))
        panel.paste(top, (10, 10))
        panel.paste(iso, (10, 640))
        d = ImageDraw.Draw(panel)
        d.text((16, 1230), f'{n}  top / iso', fill=(255, 255, 255))
        panel.save(os.path.join(OUT, f'{n}_panel.png'))
        print('panel', n)

if __name__ == '__main__':
    for n in ['Level3', 'LevelDark', 'LevelImpossible']:
        orbit_gif(n)
    showcase()
