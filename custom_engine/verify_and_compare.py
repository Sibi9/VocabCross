"""Visual audit for VocabCross.

Run after: VocabCross.exe --verify-ui
Outputs : verification_reports/compare_*.png, overview.png, summary.json
"""
import glob
import json
import os
import sys

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFont

SCREENSHOTS = "verification_screenshots"
REPORTS = "verification_reports"
W, H = 540, 960

FORBIDDEN_SOURCE = [
    "reference_ui/crossword_stage",
    "reference_ui/image_guess_stage",
    "reference_ui/wheel_stage",
    "reference_ui/ethics_island_map",
    "reference_ui/space_island_map",
    "assets/scenery/bg_crossword",
    "assets/scenery/bg_image_guess",
    "assets/scenery/bg_wheel",
    "assets/scenery/bg_ethics_island",
    "assets/scenery/bg_safety_island",
    "assets/scenery/bg_space_island",
]

PAIRS = [
    ("01_map_ethics.png", "reference_targets/ref_ethics.jpg", "Ethics Island Map"),
    ("02_map_safety.png", "reference_targets/ref_safety.jpg", "Safety First Island Map"),
    ("03_map_space.png", "reference_targets/ref_space.jpg", "Space Island Map"),
    ("04_crossword_lvl1.png", "reference_targets/ref_crossword.jpg", "Crossword Stage"),
    ("06_image_guess_lvl1.png", "reference_targets/ref_image_guess.jpg", "Image Guess Stage"),
    ("08_wheel_lvl1.png", "reference_targets/ref_wheel.jpg", "Wheel Connect Stage"),
]

ZONES = {
    "Top_HUD (0-12%)": (0.0, 0.0, 1.0, 0.12),
    "Upper_Board_Frame (12-60%)": (0.0, 0.12, 1.0, 0.60),
    "Mid_Clue_Banner (60-78%)": (0.0, 0.60, 1.0, 0.78),
    "Bottom_Tray_Controls (78-100%)": (0.0, 0.78, 1.0, 1.00),
}


def font(size=18):
    for path in ["C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/segoeui.ttf"]:
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def audit_source_code():
    violations = []
    for path in sorted(glob.glob("include/*.hpp") + glob.glob("src/*.cpp")):
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()
        for pat in FORBIDDEN_SOURCE:
            if pat in content:
                violations.append(f"{path} -> {pat}")
    if violations:
        print("[ANTI-CHEAT FAILURE] Static reference UI detected in source:")
        for v in violations:
            print("   -", v)
        sys.exit(1)
    print("[ANTI-CHEAT PASS] No forbidden reference-UI texture is loaded by the engine.")


def load(name):
    if not os.path.exists(name):
        print("Missing:", name)
        sys.exit(1)
    return Image.open(name).convert("RGB")


def histogram_similarity(a, b):
    ha = np.array(a.histogram(), dtype=np.float32)
    hb = np.array(b.histogram(), dtype=np.float32)
    ha /= ha.sum() + 1e-7
    hb /= hb.sum() + 1e-7
    return float(np.minimum(ha, hb).sum() * 100.0)


def mae(a, b):
    arr = np.abs(np.asarray(a, dtype=np.float32) - np.asarray(b, dtype=np.float32))
    return float(arr.mean())


def panel(actual_name, ref_path, title):
    act = load(os.path.join(SCREENSHOTS, actual_name)).resize((W, H), Image.Resampling.LANCZOS)
    ref = load(ref_path).resize((W, H), Image.Resampling.LANCZOS)
    diff = ImageChops.difference(act, ref)
    arr = np.asarray(diff, dtype=np.float32).mean(axis=2)
    heat = np.clip(arr * 4.0, 0, 255).astype(np.uint8)
    heat_rgb = np.stack([heat, (heat * 0.35).astype(np.uint8), (heat * 0.10).astype(np.uint8)], axis=2)
    heat_img = Image.fromarray(heat_rgb, "RGB")

    canvas = Image.new("RGB", (W * 3 + 40, H + 60), (15, 23, 42))
    canvas.paste(ref, (10, 50))
    canvas.paste(act, (W + 20, 50))
    canvas.paste(heat_img, (W * 2 + 30, 50))
    d = ImageDraw.Draw(canvas)
    f = font(18)
    d.text((10, 15), f"REFERENCE: {title}", font=f, fill=(251, 191, 36))
    d.text((W + 20, 15), f"ENGINE RENDER: {actual_name}", font=f, fill=(56, 189, 248))
    d.text((W * 2 + 30, 15), "DIFF HEATMAP (x4)", font=f, fill=(244, 63, 94))
    out = os.path.join(REPORTS, "compare_" + actual_name)
    canvas.save(out)

    print(f"\n=== Visual audit: {title} ({actual_name}) ===")
    result = {}
    for zname, (x0, y0, x1, y1) in ZONES.items():
        box = (int(x0 * W), int(y0 * H), int(x1 * W), int(y1 * H))
        sim = histogram_similarity(act.crop(box), ref.crop(box))
        err = mae(act.crop(box), ref.crop(box))
        result[zname] = {"hist_similarity": round(sim, 2), "mae": round(err, 2)}
        print(f"  - {zname}: color/tone match {sim:5.1f}% | MAE {err:6.1f}")
    return result


def dynamic_diff(a_name, b_name, label, threshold=4.0):
    a = np.asarray(load(os.path.join(SCREENSHOTS, a_name)).resize((W, H)), dtype=np.int16)
    b = np.asarray(load(os.path.join(SCREENSHOTS, b_name)).resize((W, H)), dtype=np.int16)
    d = np.abs(a - b).mean(axis=2)
    changed = float((d > 12).mean() * 100.0)
    mean = float(d.mean())
    ok = mean > threshold and changed > 2.0
    print(f"  - {label}: mean diff {mean:.2f}, changed pixels {changed:.2f}% -> {'PASS' if ok else 'FAIL'}")
    return {"mean_diff": round(mean, 2), "changed_percent": round(changed, 2), "pass": ok}


def overview():
    names = [p for p in sorted(glob.glob(os.path.join(SCREENSHOTS, "*.png")))]
    names = [p for p in names if os.path.basename(p)[:2] in {"01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"}]
    cols, thumb_w, thumb_h = 5, 270, 480
    rows = (len(names) + cols - 1) // cols
    canvas = Image.new("RGB", (cols * (thumb_w + 12) + 12, rows * (thumb_h + 52) + 64), (10, 17, 34))
    d = ImageDraw.Draw(canvas)
    f = font(14)
    for i, name in enumerate(names):
        im = Image.open(name).convert("RGB").resize((thumb_w, thumb_h), Image.Resampling.LANCZOS)
        x = 12 + (i % cols) * (thumb_w + 12)
        y = 64 + (i // cols) * (thumb_h + 52)
        canvas.paste(im, (x, y))
        d.text((x, y - 22), os.path.basename(name), font=f, fill=(251, 191, 36))
    d.text((12, 12), "VocabCross - real engine screenshots (540x960) captured with --verify-ui",
           font=font(18), fill=(226, 232, 240))
    canvas.save(os.path.join(REPORTS, "overview.png"))
    print("\n[OK] overview.png")


def arabic_check():
    path = os.path.join(SCREENSHOTS, "09_arabic_victory_modal.png")
    im = load(path)
    # green translation card bounds in the victory modal
    crop = im.crop((52, 382, 488, 492))
    arr = np.asarray(crop)
    white_glyphs = (arr[:, :, 0] > 180) & (arr[:, :, 1] > 180) & (arr[:, :, 2] > 180)
    ratio = float(white_glyphs.mean())
    ok = ratio > 0.01
    print(f"  - Arabic translation card: bright glyph coverage {ratio*100:.2f}% -> {'PASS' if ok else 'FAIL'}")
    return {"glyph_coverage_percent": round(ratio * 100, 2), "pass": ok}


def main():
    os.makedirs(REPORTS, exist_ok=True)
    print("=== 1. Source anti-cheat audit ===")
    audit_source_code()

    print("\n=== 2. Screenshot presence / dimensions ===")
    for name in ["01_map_ethics.png", "02_map_safety.png", "03_map_space.png",
                 "04_crossword_lvl1.png", "05_crossword_lvl2.png",
                 "06_image_guess_lvl1.png", "07_image_guess_lvl2.png",
                 "08_wheel_lvl1.png", "09_arabic_victory_modal.png",
                 "10_map_feelings.png", "11_wheel_lvl2.png", "12_arabic_shaping.png"]:
        im = load(os.path.join(SCREENSHOTS, name))
        print(f"  - {name}: {im.width}x{im.height} {'OK' if (im.width, im.height) == (W, H) else 'BAD SIZE'}")

    print("\n=== 3. Reference comparison panels ===")
    scores = {}
    for actual, ref, title in PAIRS:
        scores[actual] = panel(actual, ref, title)

    print("\n=== 4. Dynamic mutation proof ===")
    dynamic = {
        "crossword_04_vs_05": dynamic_diff("04_crossword_lvl1.png", "05_crossword_lvl2.png", "Crossword level 1 vs level 2"),
        "image_06_vs_07": dynamic_diff("06_image_guess_lvl1.png", "07_image_guess_lvl2.png", "Image guess level 1 vs level 2"),
        "wheel_08_vs_11": dynamic_diff("08_wheel_lvl1.png", "11_wheel_lvl2.png", "Wheel level 1 vs level 2"),
    }

    print("\n=== 5. Arabic reveal check ===")
    arabic = arabic_check()

    overview()
    summary = {
        "anti_cheat": "PASS",
        "pair_scores": scores,
        "dynamic": dynamic,
        "arabic": arabic,
    }
    with open(os.path.join(REPORTS, "summary.json"), "w", encoding="utf-8") as f:
        json.dump(summary, f, ensure_ascii=False, indent=2)
    print("\n[OK] summary.json written")


if __name__ == "__main__":
    main()
