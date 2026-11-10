#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
One-Click UI Assets Integration & Build Script for Sibi9/VocabCross.
1. Locates the `ui assest` (or `ui_assets`) folder and automatically extracts any `.zip` packs inside it.
2. Copies all updated C++ headers and `src/main.cpp` into `custom_engine/`.
3. Compiles `VocabCross.exe` via `build.bat` and runs `VocabCross.exe --verify-ui`.
"""

import glob
import os
import shutil
import subprocess
import sys
import zipfile

def find_repo_root():
    candidates = [".", "..", "VocabCross"]
    for c in candidates:
        if os.path.isdir(os.path.join(c, "custom_engine")):
            return os.path.abspath(c)
    if os.path.isdir("include") and os.path.isdir("src"):
        return os.path.abspath("..")
    return os.path.abspath(".")

def prepare_ui_assets(repo_root):
    engine_dir = os.path.join(repo_root, "custom_engine")
    search_dirs = [
        os.path.join(repo_root, "ui assest"),
        os.path.join(engine_dir, "ui assest"),
        os.path.join(engine_dir, "assets", "ui assest"),
        os.path.join(repo_root, "ui_assets"),
        os.path.join(engine_dir, "ui_assets"),
    ]
    found_dir = None
    for d in search_dirs:
        if os.path.isdir(d):
            found_dir = d
            break

    if not found_dir:
        print("[WARN] Folder 'ui assest' not found yet. Creating custom_engine/ui assest/ ...")
        found_dir = os.path.join(engine_dir, "ui assest")
        os.makedirs(found_dir, exist_ok=True)
        return found_dir

    print(f"[INFO] Found UI assets directory: {found_dir}")
    # Auto-extract any .zip files inside `ui assest` if not yet extracted
    for root, _, files in os.walk(found_dir):
        for f in files:
            if f.lower().endswith(".zip"):
                zp = os.path.join(root, f)
                target_sub = os.path.join(root, os.path.splitext(f)[0])
                if not os.path.isdir(target_sub):
                    print(f"[UNZIP] Extracting {f} -> {target_sub} ...")
                    with zipfile.ZipFile(zp, "r") as zf:
                        zf.extractall(target_sub)

    # Ensure custom_engine/ui assest also resolves if placed at repo root
    engine_ui_dir = os.path.join(engine_dir, "ui assest")
    if os.path.abspath(found_dir) != os.path.abspath(engine_ui_dir) and not os.path.exists(engine_ui_dir):
        try:
            shutil.copytree(found_dir, engine_ui_dir, dirs_exist_ok=True)
            print(f"[SYNC] Synced '{found_dir}' -> '{engine_ui_dir}'")
        except Exception as e:
            print(f"[NOTE] Relative path '../ui assest' will be used directly ({e}).")
    return found_dir

def install_cpp_files(repo_root):
    patch_root = os.path.dirname(os.path.abspath(__file__))
    src_engine = os.path.join(patch_root, "custom_engine")
    dst_engine = os.path.join(repo_root, "custom_engine")

    if not os.path.isdir(src_engine) or os.path.abspath(src_engine) == os.path.abspath(dst_engine):
        print("[INFO] Running directly inside custom_engine workspace.")
        return

    for sub in ["include", "src"]:
        s_dir = os.path.join(src_engine, sub)
        d_dir = os.path.join(dst_engine, sub)
        os.makedirs(d_dir, exist_ok=True)
        for fname in os.listdir(s_dir):
            shutil.copy2(os.path.join(s_dir, fname), os.path.join(d_dir, fname))
            print(f"[UPDATED] custom_engine/{sub}/{fname}")

    ver_script = os.path.join(src_engine, "verify_and_compare.py")
    if os.path.isfile(ver_script):
        shutil.copy2(ver_script, os.path.join(dst_engine, "verify_and_compare.py"))
        print("[UPDATED] custom_engine/verify_and_compare.py")

def main():
    repo_root = find_repo_root()
    print(f"=== VocabCross UI Assets Integrator (Root: {repo_root}) ===")
    prepare_ui_assets(repo_root)
    install_cpp_files(repo_root)

    engine_dir = os.path.join(repo_root, "custom_engine")
    if os.name == "nt" and os.path.isfile(os.path.join(engine_dir, "build.bat")):
        print("[BUILD] Running build.bat ...")
        subprocess.run(["cmd.exe", "/c", "build.bat"], cwd=engine_dir, check=False)
        exe_path = os.path.join(engine_dir, "VocabCross.exe")
        if os.path.isfile(exe_path):
            print("[VERIFY] Running VocabCross.exe --verify-ui ...")
            subprocess.run([exe_path, "--verify-ui"], cwd=engine_dir, check=False)
            subprocess.run([sys.executable, "verify_and_compare.py"], cwd=engine_dir, check=False)

    print("=== Integration Complete! ===")

if __name__ == "__main__":
    main()
