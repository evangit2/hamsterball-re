#!/usr/bin/env python3
"""Stitch original + reimpl screenshots side-by-side."""
import sys, os, subprocess, tempfile, glob
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
    # Use explicit paths for this batch
    comparisons = [
        ("/home/evan/hamsterball-re/reimpl/reference-screenshots/original/06_warmup2.png",
         "/tmp/hb_reimpl_L1.png", "/tmp/compare_L1.png"),
        ("/home/evan/hamsterball-re/reimpl/reference-screenshots/original/05_level2_intermediate.png",
         "/tmp/hb_reimpl_L2_v2.png", "/tmp/compare_L2.png"),
        # Level3 — need original gameplay screenshot
    ]
    for orig, reimpl, out in comparisons:
        if os.path.exists(orig) and os.path.exists(reimpl):
            stitch(orig, reimpl, out)
        else:
            print(f"Missing: {orig} or {reimpl}")
