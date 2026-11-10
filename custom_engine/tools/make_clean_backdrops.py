"""Create UI-free environment backdrops for the island maps.

The reference screenshots remain the *visual targets* (reference_targets/), but the
game never loads them.  The images written here are scenery-only: every baked HUD,
stage node, number, star, lock, button, label and wooden sign lettering has been
erased with masked inpainting.  All interactive widgets are rendered procedurally
at runtime by the C++ engine.
"""
import cv2
import numpy as np
import os

SRC = 'reference_targets'
OUT = 'assets/scenery'


def read(name):
    img = cv2.imread(os.path.join(SRC, name), cv2.IMREAD_COLOR)
    return cv2.resize(img, (540, 960), interpolation=cv2.INTER_LANCZOS4)


def ellipse(mask, x, y, rx, ry):
    cv2.ellipse(mask, (int(x), int(y)), (int(rx), int(ry)), 0, 0, 360, 255, -1)


def rect(mask, x0, y0, x1, y1):
    cv2.rectangle(mask, (int(x0), int(y0)), (int(x1), int(y1)), 255, -1)


def inpaint(img, mask):
    # Slight dilation gives the algorithm clean boundary pixels to work from.
    m = cv2.dilate(mask, np.ones((5, 5), np.uint8), iterations=1)
    return cv2.inpaint(img, m, 9, cv2.INPAINT_NS)


# ---------------------------------------------------------------- ethics map
ethics = read('ref_ethics.jpg')
m = np.zeros(ethics.shape[:2], np.uint8)
ETHICS_NODES = [
    (270, 768), (348, 742), (384, 680), (338, 618),
    (280, 590), (228, 548), (298, 488), (262, 420),
]
for i, (x, y) in enumerate(ETHICS_NODES):
    ellipse(m, x, y, 34, 32)
    # stars below the node + small padlock / check badge
    rect(m, x - 26, y + 27, x + 26, y + 54)
# bottom wooden sign lettering (the board itself is scenery, the lettering is UI)
rect(m, 178, 845, 408, 938)
ethics_clean = inpaint(ethics, m)
cv2.imwrite(os.path.join(OUT, 'clean_ethics.png'), ethics_clean)
cv2.imwrite(os.path.join(OUT, 'mask_ethics.png'), m)

# ---------------------------------------------------------------- safety map
safety = read('ref_safety.jpg')
m = np.zeros(safety.shape[:2], np.uint8)
SAFETY_NODES = [
    (267, 858), (363, 787), (339, 713), (330, 640), (247, 590),
    (217, 530), (238, 462), (278, 408), (271, 345), (273, 239),
]
for i, (x, y) in enumerate(SAFETY_NODES):
    ellipse(m, x, y, 32, 27)
    rect(m, x - 26, y + 22, x + 26, y + 45)
safety_clean = inpaint(safety, m)
cv2.imwrite(os.path.join(OUT, 'clean_safety.png'), safety_clean)
cv2.imwrite(os.path.join(OUT, 'mask_safety.png'), m)

# ---------------------------------------------------------------- space map
space = read('ref_space.jpg')
m = np.zeros(space.shape[:2], np.uint8)
SPACE_NODES = [
    (268, 835), (160, 712), (270, 615), (370, 538),
    (396, 438), (309, 398), (232, 318),
]
for x, y in SPACE_NODES:
    ellipse(m, x, y, 31, 26)
    rect(m, x - 28, y + 20, x + 28, y + 46)
# "Cleared" label + 3 baked stars above node 01
rect(m, 220, 860, 318, 900)
rect(m, 226, 774, 316, 814)
# baked green check floating above node 02
ellipse(m, 160, 680, 24, 24)
# top HUD: star/progress capsule, coin capsule, menu button
rect(m, 8, 8, 292, 72)
rect(m, 312, 10, 460, 64)
rect(m, 468, 6, 532, 72)
# bottom settings / map buttons
rect(m, 9, 868, 72, 932)
rect(m, 467, 868, 532, 932)
space_clean = inpaint(space, m)
cv2.imwrite(os.path.join(OUT, 'clean_space.png'), space_clean)
cv2.imwrite(os.path.join(OUT, 'mask_space.png'), m)

print('clean backdrops written to', OUT)
for f in ['clean_ethics.png', 'clean_safety.png', 'clean_space.png']:
    p = os.path.join(OUT, f)
    print(f, cv2.imread(p).shape, os.path.getsize(p))
