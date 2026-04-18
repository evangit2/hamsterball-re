#!/usr/bin/env python3
"""Stitch original + reimpl screenshots side-by-side for vision model comparison."""
import sys
import os
from PIL import Image

def stitch(original_path, reimpl_path, output_path, label_orig="ORIGINAL", label_reimpl="REIMPL"):
    """Create side-by-side image with labels."""
    orig = Image.open(original_path)
    reimpl = Image.open(reimpl_path)
    
    # Resize both to same height (use the smaller)
    target_h = min(orig.height, reimpl.height)
    if orig.height != target_h:
        orig = orig.resize((int(orig.width * target_h / orig.height), target_h), Image.LANCZOS)
    if reimpl.height != target_h:
        reimpl = reimpl.resize((int(reimpl.width * target_h / reimpl.height), target_h), Image.LANCZOS)
    
    # Create combined image with a small gap
    gap = 4
    total_w = orig.width + gap + reimpl.width
    combined = Image.new("RGB", (total_w, target_h), (0, 0, 0))
    combined.paste(orig, (0, 0))
    combined.paste(reimpl, (orig.width + gap, 0))
    
    combined.save(output_path)
    print(f"Stitched: {output_path} ({total_w}x{target_h})")
    return output_path

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: stitch_compare.py <original.png> <reimpl.png> <output.png>")
        sys.exit(1)
    stitch(sys.argv[1], sys.argv[2], sys.argv[3])