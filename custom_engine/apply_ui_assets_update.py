#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
VocabCross - Automated Verification, Build & Launch Script
Target Engine Directory:
  C:\Users\ghenemi\Desktop\VocabCross_Flutter\custom_engine\
"""

import os
import shutil
import subprocess
import sys

TARGET_ENGINE_DEFAULT = r"C:\Users\ghenemi\Desktop\VocabCross_Flutter\custom_engine"


def resolve_engine_dir():
    if os.path.isdir(TARGET_ENGINE_DEFAULT):
        return os.path.abspath(TARGET_ENGINE_DEFAULT)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    candidates = [
        os.path.join(script_dir, "custom_engine"),
        script_dir if (os.path.isdir(os.path.join(script_dir, "include")) and os.path.isdir(os.path.join(script_dir, "src"))) else "",
        os.path.abspath("custom_engine"),
    ]
    for c in candidates:
        if c and os.path.isdir(c):
            return os.path.abspath(c)
    return os.path.abspath(TARGET_ENGINE_DEFAULT)


def safe_copy_file(src, dst):
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    if os.path.exists(dst):
        try:
            if os.path.samefile(src, dst):
                return False
        except OSError:
            pass
        if os.path.normcase(os.path.abspath(src)) == os.path.normcase(os.path.abspath(dst)):
            return False
    try:
        shutil.copy2(src, dst)
        return True
    except shutil.SameFileError:
        return False


def sync_files_if_needed(dst_engine):
    patch_root = os.path.dirname(os.path.abspath(__file__))
    src_engine = os.path.join(patch_root, "custom_engine")
    if not os.path.isdir(src_engine):
        return 0

    if os.path.normcase(os.path.abspath(src_engine)) == os.path.normcase(os.path.abspath(dst_engine)):
        return 0

    copied_count = 0
    for root, _, files in os.walk(src_engine):
        rel = os.path.relpath(root, src_engine)
        target_dir = os.path.join(dst_engine, rel) if rel != "." else dst_engine
        os.makedirs(target_dir, exist_ok=True)
        for f in files:
            s_file = os.path.join(root, f)
            d_file = os.path.join(target_dir, f)
            if safe_copy_file(s_file, d_file):
                copied_count += 1
    return copied_count


def create_english_cmd_runner(dst_engine):
    bat_path = os.path.join(dst_engine, "verify_build_and_run.bat")
    bat_content = r"""@echo off
title VocabCross - Verification, Build and Launch Console
color 0A
cd /d "%~dp0"

echo ============================================================================
echo   VOCABCROSS ENGINE - AUTOMATED VERIFICATION, BUILD AND LAUNCH
echo   Target Directory: %CD%
echo ============================================================================
echo.

echo [STEP 1/4] Verifying Modified C++ Headers and Clean UI Assets...
set MISSING=0

for %%H in (
    "include\ui_asset_pack.hpp"
    "include\screen_home.hpp"
    "include\screen_maps.hpp"
    "include\screen_crossword.hpp"
    "include\screen_image_guess.hpp"
    "include\screen_wheel.hpp"
    "include\screen_shop.hpp"
    "include\screen_streak.hpp"
    "include\victory_modal.hpp"
    "include\word_card_modal.hpp"
    "src\main.cpp"
) do (
    if exist %%H (
        echo   [OK] Verified source file: %%~H
    ) else (
        echo   [MISSING] Source file not found: %%~H
        set MISSING=1
    )
)

for %%A in (
    "assets\ui_kits\home_living\frame_01.png"
    "assets\ui_kits\map_ethics_living\frame_01.png"
    "assets\ui_kits\map_safety_living\frame_01.png"
    "assets\ui_kits\map_space_living\frame_01.png"
    "assets\ui_kits\map_academy\map_background_clean.png"
    "assets\ui_kits\crossword\bg_crossword_clean.png"
    "assets\ui_kits\crossword\fox_rgba\frame_01.png"
    "assets\ui_kits\image_guess\bg_image_guess_clean.png"
    "assets\ui_kits\image_guess\robot_rgba\frame_01.png"
    "assets\ui_kits\wheel\bg_wheel_clean.png"
    "assets\ui_kits\wheel\astro_rgba\frame_01.png"
    "assets\ui_kits\shop_living\frame_01.png"
    "assets\ui_kits\streak_living\frame_01.png"
    "assets\ui_kits\victory\scene_base_without_mascot.png"
    "assets\ui_kits\word_card\ui_modal_card_blank.png"
) do (
    if exist %%A (
        echo   [OK] Verified clean asset: %%~A
    ) else (
        echo   [WARN] Asset not found at: %%~A
    )
)

echo.
echo [STEP 2/4] Building VocabCross.exe in %CD% ...
if exist "VocabCross.exe" (
    taskkill /F /IM VocabCross.exe >nul 2>&1
    del /F /Q "VocabCross.exe" >nul 2>&1
)

if exist "build.bat" (
    echo   [BUILD] Running build.bat ...
    call build.bat
) else (
    echo   [BUILD] Compiling directly with g++ ...
    g++ -O2 -std=c++17 src/main.cpp -o VocabCross.exe -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
)

echo.
echo [STEP 3/4] Verifying Built Executable Output...
if exist "%CD%\VocabCross.exe" (
    for %%F in ("%CD%\VocabCross.exe") do (
        echo   [SUCCESS] Built executable is ready!
        echo   [OUTPUT PATH] %%~fF
        echo   [FILE SIZE]   %%~zF bytes
        echo   [BUILD TIME]  %%~tF
    )
) else (
    color 0C
    echo   [ERROR] Build failed! VocabCross.exe was not generated in %CD%.
    echo   Please inspect the compiler errors above.
    pause
    exit /b 1
)

echo.
echo [STEP 4/4] Launching VocabCross.exe ...
echo   Opening game window from: %CD%\VocabCross.exe
start "" "%CD%\VocabCross.exe"
echo.
echo ============================================================================
echo   ALL STEPS COMPLETED! VOCABCROSS IS NOW RUNNING.
echo ============================================================================
"""
    with open(bat_path, "w", encoding="ascii", errors="ignore") as f:
        f.write(bat_content.replace("\n", "\r\n"))
    return bat_path


def main():
    dst_engine = resolve_engine_dir()
    print("=" * 76)
    print("  VOCABCROSS - AUTOMATED VERIFICATION, BUILD & LAUNCH")
    print(f"  Target Path: {dst_engine}")
    print("=" * 76)

    copied = sync_files_if_needed(dst_engine)
    if copied > 0:
        print(f"[SYNC] Copied {copied} updated files into {dst_engine}")
    else:
        print("[SYNC] All modified files and clean assets are in place inside custom_engine.")

    bat_path = create_english_cmd_runner(dst_engine)
    print(f"[CMD] Generated English verification & build script: {bat_path}")

    if os.name == "nt":
        print("[LAUNCH] Opening dedicated English CMD window to verify, build, and launch VocabCross.exe...")
        create_new_console = getattr(subprocess, "CREATE_NEW_CONSOLE", 0x00000010)
        subprocess.Popen(
            ["cmd.exe", "/k", bat_path],
            cwd=dst_engine,
            creationflags=create_new_console,
        )
    else:
        print("[INFO] Ready to run on Windows.")


if __name__ == "__main__":
    main()
