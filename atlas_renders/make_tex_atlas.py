#!/usr/bin/env python3
"""Textured atlas + orbit GIFs using render2 (z-buffer, real textures)."""
import sys, os, math, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render2 import Scene, load_tex
sys.path.insert(0, '/home/evan/.hermes/skills/reverse-engineering/hamsterball-meshworld/scripts')
from meshworld_parser import MeshWorldFile
from PIL import Image, ImageDraw

LVLDIR = '/home/evan/hamsterball-re/originals/installed/extracted/Levels'
OUT = '/home/evan/hamsterball-re/atlas_renders'

RACE = ['Level1','Level2','Level3','Level4','Level5','Level6','Level7','Level8',
        'Level9','Level10','LevelCascade','LevelUp','LevelDark','LevelGlass','LevelImpossible']
ARENA = ['Arena-Beginner','Arena-Intermediate','Arena-WarmUp','Arena-Dizzy','Arena-Toob',
         'Arena-Tower','Arena-Expert','Arena-Neon','Arena-Sky','Arena-Wobbly','Arena-Odd',
         'Arena-Up','Arena-Glass','Arena-Master','Arena-Impossible']

def find(name):
    return os.path.join(LVLDIR, name + '.MESHWORLD')

def render_iso(name, size=760, yaw=math.radians(45), pitch=math.radians(32)):
    mw = MeshWorldFile.parse(find(name))
    sc = Scene(mw, size)
    return sc.render(yaw, pitch)

def orbit_gif(name, pitch_deg=30, size=560, frames=40, dur=85):
    mw = MeshWorldFile.parse(find(name))
    sc = Scene(mw, size)
    fr = []
    for i in range(frames):
        yaw = math.radians(i * 360 / frames)
        fr.append(sc.render(yaw, math.radians(pitch_deg)))
    path = os.path.join(OUT, f'{name}_tex_orbit.gif')
    fr[0].save(path, save_all=True, append_images=fr[1:], duration=dur, loop=0, optimize=False)
    print(f'{name}: {len(fr)} frames')
    return path

if __name__ == '__main__':
    os.makedirs(os.path.join(OUT, 'tex'), exist_ok=True)
    # 1) textured iso of all 30
    for name in RACE + ARENA:
        try:
            img = render_iso(name)
        except Exception as e:
            print(f'!! {name}: {e}')
            continue
        img.save(os.path.join(OUT, 'tex', name + '_tex.png'))
        print(f'{name}: {img.size}')
    # 2) orbit GIFs of the showcase levels
    for n in ['Level1', 'LevelDark', 'LevelImpossible', 'LevelUp']:
        orbit_gif(n)
    print('done')
