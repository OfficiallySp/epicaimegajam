"""Turn the raw gpt-image-2 surface images into tileable detail textures for M_Surface.

Run:  python tools/make_surfaces.py      (needs Pillow and numpy)
In:   assets/src/surfaces/raw/{wood,scenic,stone}.png  (prompts and task ids in assets/src/surfaces/README.md)
Out:  assets/src/surfaces/T_Surface_{Wood,Scenic,Stone}.png, imported by Content/Python/mj_setup.py (import_surfaces)
      assets/src/surfaces/contact.png (each texture tiled 2x2, to check the seams)

Each texture is made seamless (blended with a copy shifted by half, the copy showing at the borders), then scaled in
linear light so every channel averages 0.5. The material multiplies by 2 and by the reel's colour, so a surface keeps
its old average colour and the texture only adds grain. Contrast is capped so nothing gets loud.
"""
import os

import numpy as np
from PIL import Image

SIZE = 1024
HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "..", "assets", "src", "surfaces")
NAMES = {"wood": "Wood", "scenic": "Scenic", "stone": "Stone"}
MAX_SPREAD = {"wood": 0.55, "scenic": 0.35, "stone": 0.30}  # max std-dev of the detail around 1.0 (after the x2)


def to_linear(c):
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


def to_srgb(c):
    c = np.clip(c, 0.0, 1.0)
    return np.where(c <= 0.0031308, c * 12.92, 1.055 * np.power(c, 1 / 2.4) - 0.055)


def seamless(a):
    h, w = a.shape[:2]
    b = np.roll(a, (h // 2, w // 2), axis=(0, 1))
    y, x = np.mgrid[0:h, 0:w]
    d = np.maximum(np.abs(x / (w - 1) - 0.5), np.abs(y / (h - 1) - 0.5)) * 2.0  # 0 centre, 1 border
    m = np.clip((d - 0.55) / 0.4, 0.0, 1.0)
    m = (m * m * (3 - 2 * m))[..., None]
    return a * (1 - m) + b * m


def main():
    tiles = []
    for raw, name in NAMES.items():
        img = Image.open(os.path.join(SRC, "raw", raw + ".png")).convert("RGB").resize((SIZE, SIZE), Image.LANCZOS)
        lin = to_linear(np.asarray(img, dtype=np.float64) / 255.0)
        lin = seamless(lin)
        detail = lin / lin.reshape(-1, 3).mean(axis=0)  # per channel mean 1.0
        spread = detail.std()
        if spread > MAX_SPREAD[raw]:
            detail = 1.0 + (detail - 1.0) * (MAX_SPREAD[raw] / spread)
        out = to_srgb(detail * 0.5)
        tex = Image.fromarray((out * 255.0 + 0.5).astype(np.uint8))
        tex.save(os.path.join(SRC, f"T_Surface_{name}.png"))
        tiles.append(tex)
        print(f"T_Surface_{name}.png  spread {spread:.3f} -> {min(spread, MAX_SPREAD[raw]):.3f}")
    half = SIZE // 2
    sheet = Image.new("RGB", (half * 2 * len(tiles), half * 2))
    for i, t in enumerate(tiles):
        small = t.resize((half, half), Image.LANCZOS)
        for dx in (0, 1):
            for dy in (0, 1):
                sheet.paste(small, (i * 2 * half + dx * half, dy * half))
    sheet.save(os.path.join(SRC, "contact.png"))


if __name__ == "__main__":
    main()
