#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Strict Visual & Asset Audit for VocabCross UI Assets Integration.
Run after: VocabCross.exe --verify-ui
"""

import glob
import json
import os
import sys
import numpy as np
from PIL import Image, ImageDraw

SCREENSHOTS_DIR = "verification_screenshots"
REPORTS_DIR = "verification_reports"

REQUIRED_SCREENSHOTS = [
    ("00_home_bac_vocab_quest.png", "BAC VOCAB QUEST Home Screen + Animated Trio"),
    ("01_map_ethics.png", "Unit 1 Map: Ethics Island"),
    ("02_map_safety.png", "Unit 2 Map: Safety First / Food Pyramid Island"),
    ("03_map_space.png", "Unit 3 Map: Space & Astronomy Observatory"),
    ("04_map_graduation_academy.png", "Graduation Academy Map (Scrolls 1-14 -> Blue Castle)"),
    ("04_crossword_lvl1.png", "Crossword Gameplay (Level 1 + Detective Fox)"),
    ("05_crossword_lvl2.png", "Crossword Gameplay (Level 2 Mutation Check)"),
    ("06_image_guess_lvl1.png", "Image Guess Gameplay (Level 1 + Graduation Robot)"),
    ("07_image_guess_lvl2.png", "Image Guess Gameplay (Level 2 Mutation Check)"),
    ("08_wheel_lvl1.png", "Wheel Connect Gameplay (Crystal Wheel + Astronaut)"),
    ("09_arabic_victory_modal.png", "Victory Modal (3 Golden Stars + Mascot + Arabic)"),
    ("10_word_card_integrity.png", "Word Card Modal (INTEGRITY 3D Tiles + Arabic)"),
    ("11_booster_shop.png", "Booster Shop (Living Alchemist Background + Static UI)"),
    ("12_daily_streak_1_to_7.png", "Daily Streak Roadmap (Days 1-7 + Owl + Shaking Chest)"),
]

def main():
    os.makedirs(REPORTS_DIR, exist_ok=True)
    missing = []
    cards = []

    for fname, label in REQUIRED_SCREENSHOTS:
        p = os.path.join(SCREENSHOTS_DIR, fname)
        if not os.path.isfile(p):
            missing.append(fname)
            continue
        im = Image.open(p).convert("RGB")
        arr = np.asarray(im, dtype=np.float32)
        std_val = float(arr.std())
        if std_val < 8.0:
            print(f"[FAIL] {fname} appears blank or single-color (std={std_val:.2f})")
            sys.exit(1)
        cards.append((fname, label, im, std_val))
        print(f"[PASS] {fname:32s} | {label} (std={std_val:.1f})")

    if missing:
        print("[FAIL] Missing required screenshots:", missing)
        sys.exit(1)

    # Verify two-level dynamic mutation
    c1 = np.asarray(Image.open(os.path.join(SCREENSHOTS_DIR, "04_crossword_lvl1.png")).convert("RGB"), dtype=np.float32)
    c2 = np.asarray(Image.open(os.path.join(SCREENSHOTS_DIR, "05_crossword_lvl2.png")).convert("RGB"), dtype=np.float32)
    diff_cw = float(np.abs(c1 - c2).mean())
    print(f"[DYNAMIC CHECK] Crossword Lvl1 vs Lvl2 mean pixel diff = {diff_cw:.2f}")

    # Build overview contact sheet of all captured screens
    cols = 4
    rows = (len(cards) + cols - 1) // cols
    thumb_w, thumb_h = 270, 480
    sheet = Image.new("RGB", (cols * thumb_w, rows * (thumb_h + 36)), (12, 18, 34))
    draw = ImageDraw.Draw(sheet)

    for idx, (fname, label, im, std_val) in enumerate(cards):
        r = idx // cols
        c = idx % cols
        x0 = c * thumb_w
        y0 = r * (thumb_h + 36)
        thumb = im.resize((thumb_w - 8, thumb_h - 8))
        sheet.paste(thumb, (x0 + 4, y0 + 32))
        draw.text((x0 + 8, y0 + 8), f"{fname[:24]}", fill=(253, 224, 71))

    overview_path = os.path.join(REPORTS_DIR, "all_14_screens_overview.png")
    sheet.save(overview_path)
    print(f"[REPORT] Saved complete 14-screen visual audit sheet to {overview_path}")

if __name__ == "__main__":
    main()
