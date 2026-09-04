#!/bin/bash
# Generate the eight billboard sprites for r02: two subjects (tree, rock),
# each at all four VDP hardware sprite sizes (8x8, 16x16, 24x24, 32x32).
#
# Unlike r01's single 128x128 source scaled at render time, each of these
# is a SEPARATE image, hand-fitted to its own pixel size -- the VDP has no
# "scale this texture" step, so what you draw at 8x8 has to look right at
# 8x8, not just be a shrunk copy of the 32x32 one.
#
# 8bpp indexed PNG, 3 colours: index 0 transparent, the rest the subject's
# two real colours -- comfortably inside the Mega Drive's 15 usable
# colours per palette line.

set -e

mkdir -p res/image

python3 - <<'EOF'
from PIL import Image

SIZES = [8, 16, 24, 32]

# One shared palette across every sprite -- tree and rock alike -- so any
# ONE SpriteDefinition's bundled palette is correct for all eight; only
# one PAL_setPalette() call is needed at startup, not one per subject.
# index 0 transparent, 1 trunk, 2 leaves, 3 rock, 4 sky, 5 ground -- the
# last two are never drawn into these sprite images (render.c's own
# background tiles reference them directly) but have to live in this
# same palette, since only one PAL1 load happens all chapter.
PALETTE = [
    0, 0, 0,
    0x5a, 0x3a, 0x1e,
    0x2e, 0x8b, 0x57,
    0x7a, 0x7a, 0x7a,
    0x5c, 0x9d, 0xe8,
    0x4a, 0x4a, 0x4a,
]

def indexed(size):
    img = Image.new("P", (size, size))
    img.putpalette(PALETTE + [0, 0, 0] * (256 - len(PALETTE) // 3))
    img.info["transparency"] = 0
    return img

def draw_tree(size):
    img = indexed(size)
    px = img.load()
    trunk_w = max(1, size // 6)
    trunk_h = max(1, size // 3)
    cx = size // 2
    for y in range(size - trunk_h, size):
        for x in range(cx - trunk_w // 2, cx + trunk_w // 2 + 1):
            if 0 <= x < size:
                px[x, y] = 1
    top = size - trunk_h
    for y in range(0, top):
        half = int((y / max(1, top)) * (size // 2))
        for x in range(cx - half, cx + half + 1):
            if 0 <= x < size:
                px[x, y] = 2
    return img

def draw_rock(size):
    img = indexed(size)
    px = img.load()
    cx = size // 2
    cy = int(size * 0.6)
    rx = size // 2 - 1
    ry = int(size * 0.4)
    for y in range(size):
        for x in range(size):
            dx = (x - cx) / max(1, rx)
            dy = (y - cy) / max(1, ry)
            if dx * dx + dy * dy <= 1.0 and y >= size - int(size * 0.85):
                px[x, y] = 3
    return img

for size in SIZES:
    draw_tree(size).save(f"res/image/tree_{size}.png")
    draw_rock(size).save(f"res/image/rock_{size}.png")
    print(f"  wrote res/image/tree_{size}.png, res/image/rock_{size}.png")
EOF
