#!/usr/bin/env python3
"""Render the full official level atlas (15 race + 15 arena) + GIF of highlights."""
import sys, os, math, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from render import render_level, PREFIX_COLORS
sys.path.insert(0, '/home/evan/.hermes/skills/reverse-engineering/hamsterball-meshworld/scripts')
from meshworld_parser import MeshWorldFile
from PIL import Image, ImageDraw, ImageFont

LVLDIR = '/home/evan/hamsterball-re/originals/installed/extracted/Levels'
OUT = '/home/evan/hamsterball-re/atlas_renders'

RACE = ['Level1','Level2','Level3','Level4','Level5','Level6','Level7','Level8',
        'Level9','Level10','LevelCascade','LevelUp','LevelDark','LevelGlass','LevelImpossible']
ARENA = ['Arena-Beginner','Arena-Intermediate','Arena-WarmUp','Arena-Dizzy','Arena-Toob',
         'Arena-Tower','Arena-Expert','Arena-Neon','Arena-Sky','Arena-Wobbly','Arena-Odd',
         'Arena-Up','Arena-Glass','Arena-Master','Arena-Impossible']

def find_file(name):
    p = os.path.join(LVLDIR, name + '.MESHWORLD')
    return p if os.path.exists(p) else None

def render_to(mw, path, yaw, pitch, size):
    img = render_level(mw, yaw, pitch, size=size)
    if img is None:
        print(f'  !! no geometry for {path}')
        return None
    img.save(path)
    return img

def main():
    os.makedirs(OUT, exist_ok=True)
    # 1) top-down atlas of all 30 official levels
    sizes = []
    imgs = []
    for name in RACE + ARENA:
        p = find_file(name)
        if not p:
            print(f'!! missing {name}')
            continue
        mw = MeshWorldFile.parse(p)
        print(f'{name}: {len(mw.vertices)} verts, {len(mw.get_named_geoms())} named geoms')
        img = render_to(mw, os.path.join(OUT, f'png/{name}.png'), yaw=0.0, pitch=0.0, size=760)
        if img:
            sizes.append((name, img.size[0]))
            imgs.append((name, img))
    # 2) isometric pairs for a few interesting ones
    for name in ['Level1','Level3','Level10','LevelImpossible','Arena-Expert','Arena-Impossible']:
        p = find_file(name)
        if p:
            mw = MeshWorldFile.parse(p)
            render_to(mw, os.path.join(OUT, f'png/{name}_iso.png'), yaw=math.radians(45), pitch=math.radians(35), size=760)
    # 3) rotating GIF for Level1 (top camera slowly orbiting)
    mw = MeshWorldFile.parse(find_file('Level1'))
    frames = []
    for i in range(36):
        yaw = math.radians(i * 10)
        img = render_level(mw, yaw, pitch=math.radians(30), size=520)
        frames.append(img)
    gif = os.path.join(OUT, 'Level1_rotate.gif')
    frames[0].save(gif, save_all=True, append_images=frames[1:], duration=90, loop=0, optimize=False)
    print(f'GIF: {gif}')
    # 4) montage sheet
    cols, rows = 10, 3
    cell = 300
    sheet = Image.new('RGB', (cols*cell, rows*cell), (16, 18, 24))
    font = ImageFont.load_default()
    idx = 0
    for name, img in imgs:
        r, c = divmod(idx, cols)
        if r >= rows: break
        thumb = img.resize((cell-8, cell-8))
        sheet.paste(thumb, (c*cell+4, r*cell+4))
        d = ImageDraw.Draw(sheet)
        d.text((c*cell+8, r*cell+cell-22), name, fill=(255,255,255), font=font)
        idx += 1
    sheet.save(os.path.join(OUT, 'atlas_sheet.png'))
    print('atlas done:', len(imgs), 'levels')

if __name__ == '__main__':
    main()
