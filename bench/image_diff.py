#!/usr/bin/env python3
"""Compare two captures: prints differing pixel counts and writes an amplified diff image."""
import sys
from PIL import Image, ImageChops

a = Image.open(sys.argv[1]).convert("RGB")
b = Image.open(sys.argv[2]).convert("RGB")
if a.size != b.size:
    print(f"{sys.argv[1]}: size mismatch {a.size} vs {b.size}")
    sys.exit(1)
diff = ImageChops.difference(a, b)
gray = diff.convert("L")
hist = gray.histogram()
total = a.size[0] * a.size[1]
changed = total - hist[0]
big = sum(hist[16:])
max_diff = max(i for i, c in enumerate(hist) if c) if changed else 0
print(f"{sys.argv[1].rsplit('/', 1)[-1]:24s} changed px: {changed:8d} ({changed / total * 100:6.3f}%)  "
      f">16: {big:8d}  max diff: {max_diff}")
if len(sys.argv) > 3:
    diff.point(lambda v: min(255, v * 8)).save(sys.argv[3])
sys.exit(0 if big == 0 else 1)
