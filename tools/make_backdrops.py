"""Earlier script-painted fallback, superseded by gpt-image-2 art via assemble_backdrops.py.

Paint the backdrop atlases for the set's flats: one 2048x640 image per reel, four panels wide.

Run:  python tools/make_backdrops.py      (needs Pillow: python -m pip install pillow)
Output: assets/src/backdrops/T_Backdrop_R<1..3>.png, imported by Content/Python/mj_setup.py (import_backdrops).
Each flat on the set shows one of the four panels (picked per tile), so a wall reads as a row of different flats.
Original art, made for the jam by script; no AI image model involved.
"""
import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter

W, H, PANELS = 512, 640, 4
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "src", "backdrops")


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def vgrad(d, box, top, bottom):
    x0, y0, x1, y1 = box
    for y in range(y0, y1):
        d.line([(x0, y), (x1, y)], fill=lerp(top, bottom, (y - y0) / max(1, y1 - y0 - 1)))


def grain(img, amount, seed):
    rnd = random.Random(seed)
    px = img.load()
    for y in range(img.height):
        for x in range(img.width):
            n = rnd.randint(-amount, amount)
            r, g, b = px[x, y]
            px[x, y] = (max(0, min(255, r + n)), max(0, min(255, g + n)), max(0, min(255, b + n)))


def museum(i, rnd):
    img = Image.new("RGB", (W, H))
    d = ImageDraw.Draw(img)
    vgrad(d, (0, 0, W, H), (30, 104, 116), (20, 78, 92))
    for x in range(0, W, 64):  # damask stripes
        d.rectangle([x, 0, x + 31, H], fill=None, outline=None)
        d.rectangle([x, 0, x + 31, int(H * 0.72)], fill=lerp((30, 104, 116), (36, 116, 128), 0.6))
    d.rectangle([0, 28, W, 36], fill=(196, 160, 88))  # picture rail
    y0 = int(H * 0.72)
    vgrad(d, (0, y0, W, H), (74, 46, 26), (48, 30, 18))  # wainscot
    d.rectangle([0, y0, W, y0 + 10], fill=(120, 80, 44))
    for x in (28, 268):
        d.rectangle([x, y0 + 34, x + 216, H - 26], outline=(104, 68, 38), width=6)
    if i in (0, 2):  # a framed painting
        fx0, fy0, fx1, fy1 = (96, 96, 416, 392) if i == 0 else (136, 76, 376, 412)
        d.rectangle([fx0 + 8, fy0 + 12, fx1 + 8, fy1 + 12], fill=(12, 48, 58))  # shadow
        d.rectangle([fx0, fy0, fx1, fy1], fill=(206, 164, 72))
        d.rectangle([fx0 + 10, fy0 + 10, fx1 - 10, fy1 - 10], fill=(150, 112, 40))
        ix0, iy0, ix1, iy1 = fx0 + 22, fy0 + 22, fx1 - 22, fy1 - 22
        if i == 0:  # landscape
            vgrad(d, (ix0, iy0, ix1, iy1), (240, 190, 120), (250, 226, 170))
            d.ellipse([ix1 - 90, iy0 + 24, ix1 - 40, iy0 + 74], fill=(255, 244, 210))
            d.polygon([(ix0, iy1), (ix0, iy1 - 70), (ix0 + 90, iy1 - 130), (ix0 + 170, iy1 - 60), (ix1, iy1 - 110), (ix1, iy1)], fill=(64, 106, 96))
            d.polygon([(ix0, iy1), (ix0, iy1 - 40), (ix0 + 130, iy1 - 78), (ix1, iy1 - 36), (ix1, iy1)], fill=(38, 74, 70))
        else:  # portrait
            d.rectangle([ix0, iy0, ix1, iy1], fill=(70, 34, 40))
            cx = (ix0 + ix1) // 2
            d.ellipse([cx - 46, iy0 + 50, cx + 46, iy0 + 150], fill=(226, 190, 160))
            d.polygon([(cx - 90, iy1), (cx - 60, iy0 + 170), (cx + 60, iy0 + 170), (cx + 90, iy1)], fill=(24, 26, 44))
            d.rectangle([cx - 14, iy0 + 150, cx + 14, iy0 + 180], fill=(226, 190, 160))
        d.rectangle([(fx0 + fx1) // 2 - 36, fy1 + 26, (fx0 + fx1) // 2 + 36, fy1 + 42], fill=(206, 164, 72))  # plaque
    elif i == 1:  # wall sconce
        d.polygon([(256, 150), (216, 250), (296, 250)], fill=(255, 236, 170))
        d.rectangle([246, 250, 266, 300], fill=(196, 160, 88))
        glow = Image.new("RGB", (W, H), (0, 0, 0))
        gd = ImageDraw.Draw(glow)
        gd.ellipse([120, 40, 392, 330], fill=(90, 80, 40))
        img = Image.blend(img, Image.composite(img, img, Image.new("L", (W, H), 0)), 0)
        img = Image.fromarray(__import__("numpy").clip(__import__("numpy").asarray(img, dtype="int16") + __import__("numpy").asarray(glow.filter(ImageFilter.GaussianBlur(50)), dtype="int16"), 0, 255).astype("uint8")) if _has_numpy() else img
    else:  # velvet rope sign
        d.rectangle([196, 250, 316, 330], fill=(236, 226, 200))
        d.rectangle([206, 268, 306, 276], fill=(60, 50, 40))
        d.rectangle([206, 290, 286, 296], fill=(60, 50, 40))
        d.rectangle([206, 306, 296, 312], fill=(60, 50, 40))
    return img


def _has_numpy():
    try:
        import numpy  # noqa: F401
        return True
    except Exception:
        return False


def alley(i, rnd):
    img = Image.new("RGB", (W, H), (58, 34, 34))
    d = ImageDraw.Draw(img)
    bw, bh = 64, 28
    for row, y in enumerate(range(0, H, bh)):
        off = (bw // 2) if row % 2 else 0
        for x in range(-bw, W + bw, bw):
            base = rnd.choice([(126, 62, 48), (112, 54, 44), (138, 72, 54), (98, 48, 42)])
            t = y / H
            d.rectangle([x + off + 2, y + 2, x + off + bw - 3, y + bh - 3], fill=lerp(base, (60, 30, 44), t * 0.55))
    if i == 0:  # torn poster
        d.rectangle([128, 150, 384, 470], fill=(226, 214, 180))
        d.rectangle([144, 166, 368, 300], fill=(190, 60, 70))
        d.ellipse([216, 190, 296, 270], fill=(226, 214, 180))
        for k, y in enumerate((324, 350, 376, 402, 428)):
            d.rectangle([150, y, 362 - (k % 3) * 40, y + 10], fill=(70, 60, 60))
        d.polygon([(384, 150), (384, 210), (330, 150)], fill=(112, 54, 44))
    elif i == 2:  # pipe and valve
        d.rectangle([340, 0, 372, H], fill=(70, 74, 82))
        d.rectangle([346, 0, 352, H], fill=(110, 116, 126))
        for y in (120, 420):
            d.rectangle([332, y, 380, y + 22], fill=(52, 56, 62))
        d.ellipse([318, 250, 394, 326], outline=(170, 40, 40), width=12)
        d.line([(356, 258), (356, 318)], fill=(170, 40, 40), width=8)
        d.line([(326, 288), (386, 288)], fill=(170, 40, 40), width=8)
    elif i == 3:  # barred window, lit
        d.rectangle([150, 110, 362, 330], fill=(40, 30, 34))
        vgrad(d, (164, 124, 348, 316), (255, 214, 120), (236, 150, 70))
        for x in (210, 256, 302):
            d.rectangle([x - 5, 124, x + 5, 316], fill=(40, 30, 34))
        d.rectangle([140, 330, 372, 348], fill=(90, 84, 84))
    vgrad(d, (0, H - 70, W, H), (40, 26, 30), (26, 18, 22))  # grime at the foot of the wall
    return img


def lair(i, rnd):
    img = Image.new("RGB", (W, H))
    d = ImageDraw.Draw(img)
    vgrad(d, (0, 0, W, H), (74, 30, 36), (40, 16, 22))
    for x in (0, 256):  # steel plates with rivets
        d.rectangle([x + 6, 6, x + 250, H - 6], outline=(28, 10, 14), width=6)
        for y in range(30, H, 60):
            for rx in (x + 22, x + 234):
                d.ellipse([rx - 6, y - 6, rx + 6, y + 6], fill=(120, 60, 64))
                d.ellipse([rx - 3, y - 5, rx + 3, y - 1], fill=(170, 100, 100))
    if i == 0:  # the Baron's crest
        d.ellipse([146, 130, 366, 350], fill=(20, 8, 12))
        d.ellipse([160, 144, 352, 336], outline=(230, 60, 50), width=10)
        d.polygon([(256, 170), (316, 300), (196, 300)], fill=(230, 60, 50))
        d.polygon([(256, 214), (288, 284), (224, 284)], fill=(20, 8, 12))
    elif i == 1:  # hazard stripes
        y0 = 420
        d.rectangle([0, y0, W, y0 + 90], fill=(240, 190, 40))
        for x in range(-90, W + 90, 90):
            d.polygon([(x, y0 + 90), (x + 45, y0 + 90), (x + 135, y0), (x + 90, y0)], fill=(24, 20, 22))
    elif i == 2:  # monitor wall
        d.rectangle([96, 110, 416, 340], fill=(16, 14, 18))
        d.rectangle([112, 126, 400, 324], fill=(8, 40, 34))
        pts = [(112 + k * 12, 226 + int(60 * math.sin(k * 0.55) * math.cos(k * 0.17))) for k in range(25)]
        d.line(pts, fill=(80, 255, 170), width=5)
        for y in range(140, 320, 26):
            d.line([(112, y), (400, y)], fill=(14, 70, 58), width=2)
        for k, x in enumerate((120, 170, 220)):
            d.ellipse([x, 364, x + 26, 390], fill=[(230, 60, 50), (240, 190, 40), (80, 255, 170)][k])
    else:  # vent
        d.rectangle([146, 150, 366, 330], fill=(20, 8, 12))
        for y in range(166, 320, 26):
            d.rectangle([160, y, 352, y + 12], fill=(64, 30, 34))
    d.rectangle([0, H - 44, W, H - 36], fill=(230, 60, 50))  # red floor line
    return img


def build(name, painter, seed):
    rnd = random.Random(seed)
    atlas = Image.new("RGB", (W * PANELS, H))
    for i in range(PANELS):
        atlas.paste(painter(i, rnd), (i * W, 0))
    grain(atlas, 5, seed)
    atlas = atlas.filter(ImageFilter.GaussianBlur(0.6))
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, name)
    atlas.save(path)
    print(os.path.normpath(path), atlas.size)


if __name__ == "__main__":
    build("T_Backdrop_R1.png", museum, 11)
    build("T_Backdrop_R2.png", alley, 22)
    build("T_Backdrop_R3.png", lair, 33)
