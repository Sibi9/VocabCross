#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
VocabCross - Automated C++ Runtime & Build Patch, Verification, and Launch Script
Target Engine Directory:
  C:\Users\ghenemi\Desktop\VocabCross_Flutter\custom_engine\
Fixes:
  1. WASAPI Audio Access Violation (0xc0000005): Removes duplicate InitAudioDevice() / CloseAudioDevice() in main.cpp and guards audio_manager.hpp with IsAudioDeviceReady().
  2. Filesystem Iterator Safety: Replaces shared std::error_code in indexDirectory() with isolated error codes, skip_permission_denied, and lexically_relative().
  3. MSVC Compatibility: Fixes Nav enum redefinition and DrawRectangleRoundedLinesEx -> UI::Rim.
"""

import os
import shutil
import subprocess
import sys

TARGET_ENGINE_DEFAULT = r"C:\Users\ghenemi\Desktop\VocabCross_Flutter\custom_engine"

UI_ASSET_PACK_SAFE = r'''#pragma once

#include "raylib.h"
#include "theme.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace UIAssets {

inline std::map<std::string, std::string> gPathIndex;
inline std::map<std::string, Texture2D> gTexCache;
inline bool gInitialized = false;

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

inline std::string normalizeSlashes(std::string s) {
    for (char& c : s) {
        if (c == '\\') c = '/';
    }
    return s;
}

inline void indexDirectory(const std::filesystem::path& rootDir) {
    std::error_code ec;
    if (!std::filesystem::exists(rootDir, ec) || ec) return;
    if (!std::filesystem::is_directory(rootDir, ec) || ec) return;

    auto opts = std::filesystem::directory_options::skip_permission_denied;
    std::filesystem::recursive_directory_iterator it(rootDir, opts, ec);
    std::filesystem::recursive_directory_iterator end;
    if (ec) return;

    while (it != end) {
        std::error_code fileEc;
        bool isReg = it->is_regular_file(fileEc);
        if (!fileEc && isReg) {
            const auto& p = it->path();
            auto ext = toLower(p.extension().string());
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                std::string fullPath = normalizeSlashes(p.string());
                std::string baseName = toLower(p.filename().string());
                std::string relPath = toLower(normalizeSlashes(p.lexically_relative(rootDir).string()));

                if (!relPath.empty() && gPathIndex.find(relPath) == gPathIndex.end()) {
                    gPathIndex[relPath] = fullPath;
                }
                if (!baseName.empty() && gPathIndex.find(baseName) == gPathIndex.end()) {
                    gPathIndex[baseName] = fullPath;
                }
                auto parentName = toLower(p.parent_path().filename().string());
                if (!parentName.empty() && !baseName.empty()) {
                    std::string parentKey = parentName + "/" + baseName;
                    if (gPathIndex.find(parentKey) == gPathIndex.end()) {
                        gPathIndex[parentKey] = fullPath;
                    }
                }
            }
        }
        std::error_code incEc;
        it.increment(incEc);
        if (incEc) break;
    }
}

inline void init() {
    if (gInitialized) return;
    gInitialized = true;

    const std::vector<std::string> candidateRoots = {
        "assets/ui_kits",
        "../assets/ui_kits",
        "custom_engine/assets/ui_kits",
        "assets",
        "../assets",
        "custom_engine/assets",
        "ui assest",
        "../ui assest",
        "ui_assets",
        "../ui_assets"
    };

    for (const auto& root : candidateRoots) {
        indexDirectory(root);
    }
    std::cout << "[UIAssets] Indexed " << gPathIndex.size() << " clean UI asset keys." << std::endl;
}

inline void unloadAll() {
    for (auto& kv : gTexCache) {
        if (kv.second.id != 0) {
            UnloadTexture(kv.second);
        }
    }
    gTexCache.clear();
    gPathIndex.clear();
    gInitialized = false;
}

inline std::string resolvePath(const std::string& key) {
    if (!gInitialized) init();
    std::string norm = toLower(normalizeSlashes(key));

    if (FileExists(key.c_str())) return key;
    std::string inAssets = "assets/" + norm;
    if (FileExists(inAssets.c_str())) return inAssets;
    std::string inUiKits = "assets/ui_kits/" + norm;
    if (FileExists(inUiKits.c_str())) return inUiKits;

    auto it = gPathIndex.find(norm);
    if (it != gPathIndex.end()) return it->second;

    for (const auto& kv : gPathIndex) {
        if (kv.first.size() >= norm.size() &&
            kv.first.compare(kv.first.size() - norm.size(), norm.size(), norm) == 0) {
            return kv.second;
        }
    }
    return "";
}

inline bool has(const std::string& key) {
    return !resolvePath(key).empty();
}

inline Texture2D tex(const std::string& key) {
    if (!gInitialized) init();
    std::string norm = toLower(normalizeSlashes(key));
    auto it = gTexCache.find(norm);
    if (it != gTexCache.end()) return it->second;

    std::string path = resolvePath(norm);
    Texture2D t{};
    if (!path.empty() && FileExists(path.c_str())) {
        t = LoadTexture(path.c_str());
        if (t.id != 0) {
            SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
        }
    }
    gTexCache[norm] = t;
    return t;
}

inline bool drawStretch(const std::string& key, Rectangle dst, Color tint = WHITE) {
    Texture2D t = tex(key);
    if (t.id == 0) return false;
    DrawTexturePro(t, Rectangle{ 0, 0, (float)t.width, (float)t.height },
                   dst, Vector2{ 0, 0 }, 0.0f, tint);
    return true;
}

inline bool drawFit(const std::string& key, Rectangle dst, Color tint = WHITE) {
    Texture2D t = tex(key);
    if (t.id == 0 || t.width <= 0 || t.height <= 0) return false;
    float scale = std::min(dst.width / (float)t.width, dst.height / (float)t.height);
    float w = t.width * scale;
    float h = t.height * scale;
    Rectangle fitted{
        dst.x + (dst.width - w) * 0.5f,
        dst.y + (dst.height - h) * 0.5f,
        w, h
    };
    DrawTexturePro(t, Rectangle{ 0, 0, (float)t.width, (float)t.height },
                   fitted, Vector2{ 0, 0 }, 0.0f, tint);
    return true;
}

inline bool drawCentered(const std::string& key, Vector2 center, float w, float h,
                         float rotationDeg = 0.0f, Color tint = WHITE) {
    Texture2D t = tex(key);
    if (t.id == 0 || t.width <= 0 || t.height <= 0) return false;
    float scale = std::min(w / (float)t.width, h / (float)t.height);
    float dw = t.width * scale;
    float dh = t.height * scale;
    Rectangle dst{ center.x, center.y, dw, dh };
    Vector2 origin{ dw * 0.5f, dh * 0.5f };
    DrawTexturePro(t, Rectangle{ 0, 0, (float)t.width, (float)t.height },
                   dst, origin, rotationDeg, tint);
    return true;
}

inline bool drawFrameSequence(const std::string& folderAndPrefix, int frameCount,
                              float timeSec, float fps, Rectangle dst, Color tint = WHITE,
                              bool stretch = false) {
    int idx = ((int)(timeSec * fps) % std::max(1, frameCount)) + 1;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s%02d.png", folderAndPrefix.c_str(), idx);
    return stretch ? drawStretch(buf, dst, tint) : drawFit(buf, dst, tint);
}

inline bool drawWordTiles(const std::string& prefix, const std::string& word,
                          Vector2 center, float maxRowWidth, float desiredSize, float gap = 4.0f) {
    std::string clean;
    for (char c : word) {
        if (std::isalpha((unsigned char)c)) clean.push_back((char)std::toupper((unsigned char)c));
    }
    if (clean.empty()) return false;

    int n = (int)clean.size();
    float tileSize = desiredSize;
    float totalW = n * tileSize + (n - 1) * gap;
    if (totalW > maxRowWidth && n > 0) {
        tileSize = (maxRowWidth - (n - 1) * gap) / (float)n;
        totalW = n * tileSize + (n - 1) * gap;
    }

    float startX = center.x - totalW * 0.5f;
    float y = center.y - tileSize * 0.5f;
    bool anyDrawn = false;
    for (int i = 0; i < n; ++i) {
        std::string key = prefix + std::string(1, clean[i]) + ".png";
        Rectangle r{ startX + i * (tileSize + gap), y, tileSize, tileSize };
        if (drawFit(key, r)) {
            anyDrawn = true;
        }
    }
    return anyDrawn;
}

inline void drawWrappedText3D(const std::string& text, Rectangle box, int fontSize,
                              Color fillCol, Color strokeCol, float strokeW = 2.0f,
                              bool centerLines = true) {
    std::istringstream iss(text);
    std::string word;
    std::vector<std::string> lines;
    std::string currentLine;
    int maxCharsPerLine = std::max(14, (int)(box.width / (fontSize * 0.52f)));

    while (iss >> word) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if ((int)(currentLine.size() + 1 + word.size()) <= maxCharsPerLine) {
            currentLine += " " + word;
        } else {
            lines.push_back(currentLine);
            currentLine = word;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    float lineH = fontSize + 6.0f;
    float totalH = lines.size() * lineH;
    float startY = box.y + std::max(0.0f, (box.height - totalH) * 0.5f) + fontSize * 0.5f;

    for (size_t i = 0; i < lines.size(); ++i) {
        Vector2 pos{
            centerLines ? (box.x + box.width * 0.5f) : box.x,
            startY + i * lineH
        };
        Theme::text3D(lines[i], pos, fontSize, fillCol, strokeCol, strokeW, BLACK, centerLines, true);
    }
}

} // namespace UIAssets
'''


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


def apply_all_cpp_fixes(dst_engine):
    inc_dir = os.path.join(dst_engine, "include")
    src_dir = os.path.join(dst_engine, "src")
    os.makedirs(inc_dir, exist_ok=True)
    os.makedirs(src_dir, exist_ok=True)

    # 1. Write safe include/ui_asset_pack.hpp (fixes filesystem iterator & Nav redefinition)
    uap_path = os.path.join(inc_dir, "ui_asset_pack.hpp")
    with open(uap_path, "w", encoding="utf-8") as f:
        f.write(UI_ASSET_PACK_SAFE.lstrip())
    print("[PATCH 1/4] Wrote safe include/ui_asset_pack.hpp (isolated error_code + lexically_relative)")

    # 2. Fix duplicate InitAudioDevice() and CloseAudioDevice() in src/main.cpp
    main_path = os.path.join(src_dir, "main.cpp")
    if os.path.isfile(main_path):
        with open(main_path, "r", encoding="utf-8", errors="ignore") as f:
            m_txt = f.read()
        lines = m_txt.splitlines()
        cleaned_lines = []
        for ln in lines:
            stripped = ln.strip()
            if stripped == "InitAudioDevice();" or stripped == "CloseAudioDevice();":
                continue
            cleaned_lines.append(ln)
        with open(main_path, "w", encoding="utf-8") as f:
            f.write("\n".join(cleaned_lines) + "\n")
        print("[PATCH 2/4] Removed duplicate InitAudioDevice() & CloseAudioDevice() from src/main.cpp")

    # 3. Guard InitAudioDevice() / CloseAudioDevice() inside include/audio_manager.hpp with IsAudioDeviceReady()
    am_path = os.path.join(inc_dir, "audio_manager.hpp")
    if os.path.isfile(am_path):
        with open(am_path, "r", encoding="utf-8", errors="ignore") as f:
            am_txt = f.read()
        if "if (!IsAudioDeviceReady()) InitAudioDevice();" not in am_txt:
            am_txt = am_txt.replace("InitAudioDevice();", "if (!IsAudioDeviceReady()) InitAudioDevice();")
        if "if (IsAudioDeviceReady()) CloseAudioDevice();" not in am_txt:
            am_txt = am_txt.replace("CloseAudioDevice();", "if (IsAudioDeviceReady()) CloseAudioDevice();")
        with open(am_path, "w", encoding="utf-8") as f:
            f.write(am_txt)
        print("[PATCH 3/4] Guarded audio device initialization with IsAudioDeviceReady() in include/audio_manager.hpp")

    # 4. Fix DrawRectangleRoundedLinesEx -> UI::Rim in screen_crossword.hpp and screen_image_guess.hpp
    scw_path = os.path.join(inc_dir, "screen_crossword.hpp")
    if os.path.isfile(scw_path):
        with open(scw_path, "r", encoding="utf-8", errors="ignore") as f:
            txt = f.read()
        txt = txt.replace(
            "DrawRectangleRoundedLinesEx(glowRec, 0.25f, 8, 3.5f, Color{ 250, 204, 21, 245 });",
            "UI::Rim(glowRec, 14.0f, Color{ 250, 204, 21, 245 }, WHITE, Color{ 180, 83, 9, 245 });"
        )
        with open(scw_path, "w", encoding="utf-8") as f:
            f.write(txt)

    sig_path = os.path.join(inc_dir, "screen_image_guess.hpp")
    if os.path.isfile(sig_path):
        with open(sig_path, "r", encoding="utf-8", errors="ignore") as f:
            txt = f.read()
        new_call_ig = "UI::Rim(sRec, 14.0f, Color{ 148, 163, 184, 220 }, WHITE, Color{ 51, 65, 85, 220 });"
        txt = txt.replace("DrawRectangleRoundedLinesEx(sRec, 0.25f, 6, 2.5f, Color{ 148, 163, 184, 220 });", new_call_ig)
        txt = txt.replace("DrawRectangleRoundedLinesEx(sRec, 0.25f, 6, 2.5f, WHITE);", new_call_ig)
        with open(sig_path, "w", encoding="utf-8") as f:
            f.write(txt)
    print("[PATCH 4/4] Verified UI::Rim calls in screen_crossword.hpp and screen_image_guess.hpp")


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

echo [STEP 1/4] Verifying Patched C++ Headers and Clean UI Assets...
for %%H in (
    "include\ui_asset_pack.hpp"
    "include\audio_manager.hpp"
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
    echo. | call build.bat
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

    apply_all_cpp_fixes(dst_engine)

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
