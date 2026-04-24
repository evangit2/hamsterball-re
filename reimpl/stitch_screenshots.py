#!/usr/bin/env python3
"""Stitch original + reimpl screenshots side-by-side."""
import sys, os, subprocess, tempfile
from PIL import Image, ImageDraw, ImageFont

def stitch(left_path, right_path, out_path, left_label="ORIGINAL", right_label="REIMPL"):
    left = Image.open(left_path)
    right = Image.open(right_path)
    h = 600
    left = left.resize((int(left.width * h / left.height), h), Image.LANCZOS)
    right = right.resize((int(right.width * h / right.height), h), Image.LANCZOS)
    gap = 20
    canvas = Image.new('RGB', (left.width + right.width + gap, h + 50), (30,30,30))
    canvas.paste(left, (0, 40))
    canvas.paste(right, (left.width + gap, 40))
    draw = ImageDraw.Draw(canvas)
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20)
    except:
        font = ImageFont.load_default()
    draw.text((10, 10), left_label, fill=(255,255,255), font=font)
    draw.text((left.width + gap + 10, 10), right_label, fill=(0,255,0), font=font)
    canvas.save(out_path)
    print(f"Saved comparison: {out_path}")

if __name__ == "__main__":
    import glob
    # Auto-stitch any pairs found in /tmp
    reimpl = sorted(glob.glob("/tmp/hb_reimpl_L*.png")) + sorted(glob.glob("/tmp/hb_L*_v*.png"))
    originals = sorted(glob.glob("/tmp/hb_orig_*.png"))
    print(f"Reimpl: {reimpl}")
    print(f"Originals: {originals}")
    for r in reimpl:
        level = os.path.basename(r).split('.')[0].replace('hb_reimpl_','').replace('hb_','')
        orig = f"/tmp/hb_orig_{level}.png"
        if os.path.exists(orig):
            out = f"/tmp/hb_compare_{level}.png"
            stitch(orig, r, out)
