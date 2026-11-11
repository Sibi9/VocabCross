# دليل التوجيه الصارم والأكواد الكاملة لدمج حزم الأصول (`ui assest`) في محرك VocabCross
## المستودع المستهدف: [`Sibi9/VocabCross`](https://github.com/Sibi9/VocabCross) — C++17 / Raylib (`540×960`)

> [!IMPORTANT]
> **تعليمات إلزامية للوكيل البرمجي (AI Coding Agent):**
> 1. لقد قام المستخدم بوضع جميع حزم الواجهات والأصول المصممة داخل مجلد **`ui assest`** (أو `ui_assets`).
> 2. **يُمنع عليك الارتجال أو اختصار الأكواد أو رسم مربعات بدائية مسطحة:** جميع الأكواد البرمجية الكاملة (`C++17 / Raylib`) مكتوبة لك بالكامل في هذا الملف جاهزة للنسخ واللصق الحرفي (`Copy-Paste Verbatim`) داخل مجلد `custom_engine/`.
> 3. يتضمن النظام الجديد الملف [`custom_engine/include/ui_asset_pack.hpp`](https://github.com/Sibi9/VocabCross/tree/main/custom_engine/include) الذي يفهرس تلقائياً جميع ملفات `.png` داخل `ui assest` (سواء كانت في `VocabCross/ui assest` أو `custom_engine/ui assest` أو `custom_engine/assets/ui assest`) ويستدعيها بالاسم أو المسار النسبي مع فلترة `TEXTURE_FILTER_BILINEAR`.
> 4. بعد كتابة الملفات التسعة أدناه، يجب عليك تشغيل `build.bat` ثم تشغيل `VocabCross.exe --verify-ui` لتوليد **14 لقطة شاشة حقيقية** في `verification_screenshots/` وتشغيل `python verify_and_compare.py` لإثبات عمل كافة الشاشات والرسوم المتحركة دون أي أخطاء.

---

## 1. خريطة الربط الكاملة بين حزم مجلد `ui assest` وملفات المحرك [`custom_engine/`](https://github.com/Sibi9/VocabCross/tree/main/custom_engine)

| الواجهة / الشاشة المطلوبة | الحزمة الفرعية داخل `ui assest` | الملف البرمجي في `custom_engine/` | الأصول والحركات المدمجة (60 FPS) |
| :--- | :--- | :--- | :--- |
| **1. شاشة البداية `BAC VOCAB QUEST` مع الثلاثي المتحرك** | `bac_vocab_quest_ui_kit/` | `include/screen_home.hpp` *(جديد)* | الشعار `emblem_bac_vocab_quest_crest.png`، الجزر الطائرة الـ 3 (`island_01..03`)، الكتب السحرية الطائرة (`flying_book_01..03`)، الثلاثي المتحرك (**البومة الحكيمة** مع جناحها المتحرك، **رائد الفضاء** مع ذراعيه المفصليتين، **الثعلب المحقق**)، حروف `bac_letter_A..Z`، زر `btn_play_emerald_gold_with_text.png`، وشريط الوصول السريع السفلي. |
| **2. نافذة الفوز `Victory Modal` والتعريب** | `word_graduate_victory_ui_kit/` | [`include/victory_modal.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/victory_modal.hpp) *(تحديث شامل)* | الخلفية الضبابية `bg_blurred_word_board.png`، البطاقة `scene_base_without_mascot.png`، لافتة `header_level_complete.png`، النجوم الذهبية الثلاث `three_golden_stars.png`، حروف الكلمة ثلاثية الأبعاد `tile_A..Z.png`، **التعريب العربي الفوري** بخط `Cairo.ttf`، الشخصية الكرتونية القافزة مع قبعة التخرج الطافية (`graduation_cap_cropped.png`)، الجوائز الطافية (`reward_coins_stack.png`, `reward_magic_xp_potion.png`)، وزر فتح بطاقة الكلمة. |
| **3. بطاقة الكلمة `INTEGRITY Word Card`** | `image1_integrity_word_card_ui_kit/` | `include/word_card_modal.hpp` *(جديد)* | البطاقة `ui_modal_card_blank.png`، الشارة العلوية `header_badge_new_word.png`، تركيب الكلمة بحروف `alphabet_tiles_A_to_Z/letter_A..Z.png`، أيقونة الميزان `icon_integrity_scale_hd.png`، أزرار الصوت والمذكرة (`btn_audio_speaker.png`, `btn_notebook_blue.png`)، صناديق التعريف والمرادف والمثال والتعريب العربي، وزر `btn_got_it_orange.png`. |
| **4. متجر المعززات `Booster Shop`** | `magical_alchemist_booster_shop_ui_kit/` | `include/screen_shop.hpp` *(جديد)* | الخلفية السحرية النظيفة `bg_alchemist_study_clean.png` مع **تحريك حي في الوقت الفعلي** (خفقان إضاءة الشموع، غبار ذهبي صاعد، وتوهج القوارير) مع **ثبات تام (100%) لعناصر واجهة المتجر**: اللافتة `banner_booster_shop.png`، البطاقات الأربع `card_1..4`، وأزرار الشراء التفاعلية `btn_buy_100..250.png`. |
| **5. مسار المثابرة اليومية `Daily Streak 1–7`** | `daily_streak_roadmap_ui_kit/` | `include/screen_streak.hpp` *(جديد)* | اللوحة النظيفة `daily_streak_clean_base_no_owl_no_chest.png`، ترتيب جذوع الأيام المصحح تسلسلياً من **`DAY 1` إلى `DAY 7`** (`stump_day_1.png` .. `stump_day_7_pedestal.png`)، **البومة الأستاذة المتحركة** (`char_professor_owl_pose_a/b.png`)، **صندوق الجائزة الكبرى المهتز** (`prop_bac_mega_bonus_chest_hd.png`)، وزر `btn_claim_todays_reward.png`. |
| **6. خريطة أكاديمية التخرج `Graduation Academy 1–14`** | `image2_graduation_academy_map_assets/` | [`include/screen_maps.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_maps.hpp) *(تحديث شامل)* | الخريطة `map_background_clean.png`، جسر الكتاب المفتوح العملاق `bridge_giant_open_book.png`، برج الساعة، القصر الأزرق الكبير `building_big_blue_castle.png`، توزيع **البرقيات واللفائف المرقمة من 1 إلى 14** (`scroll_step_01..14.png`) حتى بوابة القصر الأزرق، مع **تحريك العناصر الخمسة** (الولد القافز عبر البرقيات 1←14، البومة الطائرة، الكرة الأرضية، أعلام القصر، والكتب الطائرة). |
| **7. شاشات اللعب الثلاث (`Crossword`, `Image Guess`, `Wheel`)** | `ethics_quest_crossword_ui_kit/` + `astronomy_galaxy_word_wheel_ui_kit/` + `robot_animation_studio_fixed/` | [`screen_crossword.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_crossword.hpp) / [`screen_image_guess.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_image_guess.hpp) / [`screen_wheel.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_wheel.hpp) | دمج البلاطات الخشبية ثلاثية الأبعاد `wood_tile_A..Z.png`، أقراص الحروف `token_A..Z`، العجلة البلورية `word_wheel_glowing_cyan_gold_blank.png`، وحركات الـ 60 إطاراً للثعلب المحقق والروبوت ورائد الفضاء. |

---

## 2. الخطوة الأولى للوكيل: سكريبت التجهيز والدمج الآلي (`apply_ui_assets_update.py`)

إذا كانت الحزم داخل مجلد `ui assest` ما تزال مضغوطة بصيغة `.zip` أو موزعة في مجلدات فرعية، شغّل هذا السكريبت أولاً في المجلد الرئيسي للمستودع ليقوم بفك ضغط أي ملف `.zip` داخل `ui assest` وتجهيز المسارات تلقائياً:

```python
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

    if not os.path.isdir(src_engine):
        print("[INFO] Running directly inside custom_engine.")
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
```

---

## 3. الأكواد الكاملة الجاهزة للنسخ واللصق في `custom_engine/`

### 3.1 الملف الأول: `custom_engine/include/ui_asset_pack.hpp` *(ملف جديد)*

انسخ هذا الكود بالكامل واحفظه في `custom_engine/include/ui_asset_pack.hpp`:

```cpp
#pragma once

#include "raylib.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
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
    if (!std::filesystem::exists(rootDir, ec)) return;

    for (auto it = std::filesystem::recursive_directory_iterator(rootDir, ec);
         it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) continue;
        if (!it->is_regular_file(ec)) continue;
        auto ext = toLower(it->path().extension().string());
        if (ext != ".png" && ext != ".jpg" && ext != ".jpeg") continue;

        std::string fullPath = normalizeSlashes(it->path().string());
        std::string baseName = toLower(it->path().filename().string());
        std::string relPath = toLower(normalizeSlashes(std::filesystem::relative(it->path(), rootDir, ec).string()));

        // Store by relative path, parent+filename, and basename
        gPathIndex[relPath] = fullPath;
        if (gPathIndex.find(baseName) == gPathIndex.end()) {
            gPathIndex[baseName] = fullPath;
        }
        auto parentName = toLower(it->path().parent_path().filename().string());
        if (!parentName.empty()) {
            gPathIndex[parentName + "/" + baseName] = fullPath;
        }
    }
}

inline void init() {
    if (gInitialized) return;
    gInitialized = true;

    const std::vector<std::string> candidateRoots = {
        "ui assest",
        "../ui assest",
        "../../ui assest",
        "assets/ui assest",
        "../assets/ui assest",
        "ui_assets",
        "../ui_assets",
        "assets/ui_assets",
        "assets"
    };

    for (const auto& root : candidateRoots) {
        indexDirectory(root);
    }
    std::cout << "[UIAssets] Indexed " << gPathIndex.size() << " asset keys from ui assest." << std::endl;
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
    auto it = gPathIndex.find(norm);
    if (it != gPathIndex.end()) return it->second;

    // Fallback: search by suffix match
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

inline bool drawPivoted(const std::string& key, Vector2 worldPivot, Vector2 normPivot,
                        float w, float h, float rotationDeg, Color tint = WHITE) {
    Texture2D t = tex(key);
    if (t.id == 0 || t.width <= 0 || t.height <= 0) return false;
    float scale = std::min(w / (float)t.width, h / (float)t.height);
    float dw = t.width * scale;
    float dh = t.height * scale;
    Rectangle dst{ worldPivot.x, worldPivot.y, dw, dh };
    Vector2 origin{ dw * normPivot.x, dh * normPivot.y };
    DrawTexturePro(t, Rectangle{ 0, 0, (float)t.width, (float)t.height },
                   dst, origin, rotationDeg, tint);
    return true;
}

// Plays a numbered frame sequence like "06_frames_master_fox/frame_" (01..60)
inline bool drawFrameSequence(const std::string& folderAndPrefix, int frameCount,
                              float timeSec, float fps, Rectangle dst, Color tint = WHITE) {
    int idx = ((int)(timeSec * fps) % std::max(1, frameCount)) + 1;
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s%02d.png", folderAndPrefix.c_str(), idx);
    return drawFit(buf, dst, tint);
}

// Draws any English word using the A-Z letter tile PNGs from a kit
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

} // namespace UIAssets

namespace Nav {
    constexpr int SCREEN_HOME = 4;
    constexpr int SCREEN_SHOP = 5;
    constexpr int SCREEN_STREAK = 6;
}
```

---

### 3.2 الملف الثاني: `custom_engine/include/word_card_modal.hpp` *(ملف جديد — بطاقة الكلمة INTEGRITY)*

انسخ هذا الكود بالكامل واحفظه في `custom_engine/include/word_card_modal.hpp`:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <string>

class WordCardModal {
public:
    bool active = false;
    float anim = 0.0f;
    bool savedToNotebook = false;
    WordData currentWord;

    void init() {
        currentWord.word = "INTEGRITY";
        currentWord.translation_ar = "النزاهة / الاستقامة والأمانة";
        currentWord.definition_en = "The quality of being honest and having strong moral principles.";
        currentWord.synonym_en = "Honesty, Probity, Rectitude";
        currentWord.frequency = "High Frequency - Ethics in Business";
    }

    void show(const WordData& w) {
        if (!w.word.empty()) {
            currentWord = w;
        } else {
            init();
        }
        active = true;
        anim = 0.0f;
        gAudio.playPop();
    }

    void hide() {
        active = false;
    }

    void update(float dt) {
        if (!active) return;
        anim = std::min(1.0f, anim + dt * 4.0f);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            Rectangle gotItRec{ 130, 812, 280, 74 };
            Rectangle audioRec{ 415, 215, 58, 58 };
            Rectangle noteRec{ 68, 215, 58, 58 };

            if (CheckCollisionPointRec(m, gotItRec)) {
                gAudio.playClick();
                hide();
            } else if (CheckCollisionPointRec(m, audioRec)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 444, 244 }, 24);
            } else if (CheckCollisionPointRec(m, noteRec)) {
                savedToNotebook = !savedToNotebook;
                gAudio.playPop();
                Theme::spawnCelebration({ 97, 244 }, 20);
            }
        }
    }

    void draw(int w, int h) {
        if (!active) return;

        DrawRectangle(0, 0, w, h, Color{ 8, 14, 28, (unsigned char)(215 * anim) });

        float scale = 0.88f + 0.12f * anim;
        Rectangle cardRec{
            270.0f - 245.0f * scale,
            485.0f - 420.0f * scale,
            490.0f * scale,
            840.0f * scale
        };

        // 1. Main Blank Card Container from image1_integrity_word_card_ui_kit
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_modal_card_blank.png", cardRec)) {
            UI::DropShadow(cardRec, 28.0f, 10.0f, 0.45f, 4.0f);
            UI::GradientRound(cardRec, 28.0f, Color{ 248, 250, 255, 255 }, Color{ 225, 236, 250, 255 });
            UI::Rim(cardRec, 28.0f, Color{ 245, 158, 11, 255 }, WHITE, Color{ 180, 130, 40, 255 });
        }

        // 2. Top "NEW WORD DISCOVERED" Badge
        Rectangle badgeRec{ 115, 56, 310, 68 };
        if (!UIAssets::drawFit("image1_integrity_word_card_ui_kit/header_badge_new_word.png", badgeRec)) {
            UI::GradientRound(badgeRec, 20.0f, Color{ 251, 191, 36, 255 }, Color{ 217, 119, 6, 255 });
            Theme::text3D("NEW WORD CARD", { 270, 90 }, 24, WHITE, Color{ 120, 53, 15, 255 }, 2.5f, BLACK, true, true);
        }

        // 3. 3D Alphabet Tiles (A-Z) spelling the word dynamically
        Vector2 wordCenter{ 270, 162 };
        if (!UIAssets::drawWordTiles("alphabet_tiles_a_to_z/letter_", currentWord.word, wordCenter, 420.0f, 48.0f, 4.0f)) {
            Theme::text3D(currentWord.word, wordCenter, 36, Color{ 30, 64, 175, 255 }, WHITE, 3.0f, BLACK, true, true);
        }

        // 4. Audio Speaker & Notebook Buttons + Central Illustration (Golden Scale / Concept Icon)
        Rectangle noteRec{ 68, 215, 58, 58 };
        Rectangle audioRec{ 415, 215, 58, 58 };
        UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_notebook_blue.png", noteRec);
        UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_audio_speaker.png", audioRec);

        float bob = std::sin(Theme::timeNow * 3.2f) * 5.0f;
        Rectangle iconRec{ 185, 200 + bob, 170, 155 };
        if (!UIAssets::drawFit("image1_integrity_word_card_ui_kit/icon_integrity_scale_hd.png", iconRec)) {
            UIAssets::drawFit("icon_integrity_scale_original.png", iconRec);
        }

        // 5. Arabic Translation Banner (Shaped RTL Arabic via Cairo.ttf)
        Rectangle arabBox{ 58, 362, 424, 72 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", arabBox)) {
            UI::GradientRound(arabBox, 16.0f, Color{ 254, 243, 199, 255 }, Color{ 253, 230, 138, 255 });
        }
        Theme::textArabic(currentWord.translation_ar.empty() ? "النزاهة والاستقامة" : currentWord.translation_ar,
                          { 270, 398 }, 30, Color{ 15, 23, 42, 255 }, true, true, WHITE, 2.0f);

        // 6. Section 1: Part of Speech & BAC Unit Frequency
        Rectangle sec1{ 58, 446, 424, 96 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", sec1)) {
            UI::GradientRound(sec1, 16.0f, WHITE, Color{ 241, 245, 249, 255 });
        }
        Theme::text3D("BAC STREAM & SYNONYMS:", { 80, 462 }, 17, Color{ 37, 99, 235, 255 }, WHITE, 1.5f, BLANK, false, false);
        Theme::text("Synonym: " + (currentWord.synonym_en.empty() ? "Honesty / Probity" : currentWord.synonym_en),
                    { 80, 490 }, 18, Color{ 30, 41, 59, 255 }, false, false);
        Theme::text("BAC Priority: " + (currentWord.frequency.empty() ? "High Exam Frequency" : currentWord.frequency),
                    { 80, 514 }, 16, Color{ 180, 83, 9, 255 }, false, false);

        // 7. Section 2: Curriculum Definition
        Rectangle sec2{ 58, 554, 424, 118 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", sec2)) {
            UI::GradientRound(sec2, 16.0f, WHITE, Color{ 241, 245, 249, 255 });
        }
        Theme::text3D("DEFINITION:", { 80, 570 }, 18, Color{ 217, 119, 6, 255 }, WHITE, 1.5f, BLANK, false, false);
        Theme::text(currentWord.definition_en.empty()
                        ? "The quality of being honest and having strong moral principles."
                        : currentWord.definition_en,
                    { 80, 602 }, 17, Color{ 15, 23, 42, 255 }, false, false);

        // 8. Section 3: Contextual BAC Example
        Rectangle sec3{ 58, 684, 424, 110 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", sec3)) {
            UI::GradientRound(sec3, 16.0f, WHITE, Color{ 241, 245, 249, 255 });
        }
        Theme::text3D("BAC CONTEXTUAL EXAMPLE:", { 80, 700 }, 17, Color{ 22, 163, 74, 255 }, WHITE, 1.5f, BLANK, false, false);
        Theme::text("Business leaders must act with " + currentWord.word + " and transparency.",
                    { 80, 730 }, 16, Color{ 51, 65, 85, 255 }, false, false);

        // 9. Orange CTA Button ("GOT IT!")
        Rectangle gotItRec{ 130, 812, 280, 74 };
        if (!UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_got_it_orange.png", gotItRec)) {
            UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_cta_orange_blank.png", gotItRec);
            Theme::text3D("GOT IT!", { 270, 849 }, 28, WHITE, Color{ 154, 52, 18, 255 }, 3.0f, BLACK, true, true);
        }
    }
};
```

---

### 3.3 الملف الثالث: `custom_engine/include/victory_modal.hpp` *(تحديث كامل — نافذة الفوز Victory Modal)*

استبدل محتوى الملف [`custom_engine/include/victory_modal.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/victory_modal.hpp) بالكود الكامل التالي:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "word_card_modal.hpp"

#include <algorithm>
#include <cmath>
#include <string>

class VictoryModal {
public:
    bool active = false;
    float anim = 0.0f;
    bool nextClicked = false;
    bool mapClicked = false;
    WordData currentWord;
    WordCardModal wordCardModal;

    void init() {
        wordCardModal.init();
    }

    void show(const WordData& word) {
        currentWord = word;
        active = true;
        anim = 0.0f;
        nextClicked = false;
        mapClicked = false;
        wordCardModal.hide();
        Theme::coins += 50;
        gAudio.playFanfare();
        Theme::spawnCelebration({ 270.0f, 320.0f }, 80);
    }

    void hide() {
        active = false;
        wordCardModal.hide();
    }

    void update(float dt) {
        if (!active) return;
        anim = std::min(1.0f, anim + dt * 3.2f);
        nextClicked = false;
        mapClicked = false;

        if (wordCardModal.active) {
            wordCardModal.update(dt);
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            Rectangle nextRec{ 170, 770, 290, 82 };
            Rectangle replayRec{ 72, 772, 82, 82 };
            Rectangle cardBtnRec{ 130, 695, 280, 56 };
            Rectangle mascotRec{ 170, 410, 200, 210 };

            if (CheckCollisionPointRec(m, nextRec)) {
                gAudio.playClick();
                nextClicked = true;
            } else if (CheckCollisionPointRec(m, replayRec)) {
                gAudio.playClick();
                mapClicked = true;
            } else if (CheckCollisionPointRec(m, cardBtnRec)) {
                wordCardModal.show(currentWord);
            } else if (CheckCollisionPointRec(m, mascotRec)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 270.0f, 470.0f }, 48);
            }
        }
    }

    void draw(int w, int h) {
        if (!active) return;

        // 1. Blurred Board Backdrop + Dark Overlay
        if (!UIAssets::drawStretch("word_graduate_victory_ui_kit/ui_components/bg_blurred_word_board.png",
                                   Rectangle{ 0, 0, (float)w, (float)h },
                                   Color{ 255, 255, 255, (unsigned char)(220 * anim) })) {
            DrawRectangle(0, 0, w, h, Color{ 8, 12, 26, (unsigned char)(210 * anim) });
        } else {
            DrawRectangle(0, 0, w, h, Color{ 8, 12, 26, (unsigned char)(115 * anim) });
        }

        // 2. Main Modal Card Container
        Rectangle modalRec{ 28, 70, 484, 820 };
        if (!UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/scene_base_without_mascot.png", modalRec)) {
            UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/modal_victory_card_full.png", modalRec);
        }

        // 3. Top Header "LEVEL COMPLETE!" & 3 Golden Stars with Pulse
        Rectangle headerRec{ 65, 58, 410, 105 };
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/header_level_complete.png", headerRec);

        float starPulse = 1.0f + 0.06f * std::sin(Theme::timeNow * 4.5f);
        UIAssets::drawCentered("word_graduate_victory_ui_kit/rewards_and_stars/three_golden_stars.png",
                               { 270, 188 }, 280 * starPulse, 110 * starPulse);

        // 4. Ribbon Banner ("EXCELLENT!")
        Rectangle ribbonRec{ 110, 238, 320, 68 };
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/ribbon_excellent_banner.png", ribbonRec);

        // 5. Solved English Word rendered with 3D Victory Alphabet Tiles (A-Z)
        UIAssets::drawWordTiles("word_graduate_victory_ui_kit/letter_tiles_a_z/tile_",
                                currentWord.word.empty() ? "GRADUATE" : currentWord.word,
                                { 270, 325 }, 420.0f, 52.0f, 4.0f);

        // 6. Arabic Translation Badge (Shaped RTL Arabic via Cairo.ttf)
        Rectangle arBadge{ 76, 362, 388, 56 };
        UI::GradientRound(arBadge, 18.0f, Color{ 15, 23, 42, 235 }, Color{ 30, 58, 138, 235 });
        UI::Rim(arBadge, 18.0f, Color{ 250, 204, 21, 255 }, Color{ 254, 240, 138, 255 }, Color{ 180, 83, 9, 255 });
        Theme::textArabic(currentWord.translation_ar.empty() ? "خريج / إتمام المرحلة بنجاح" : currentWord.translation_ar,
                          { 270, 390 }, 28, Color{ 254, 240, 138, 255 }, true, true, BLACK, 2.0f);

        // 7. Animated Graduate Mascot + Floating Graduation Cap + Floating Rewards
        float bounceY = std::abs(std::sin(Theme::timeNow * 4.2f)) * -14.0f;
        float capLagY = std::sin(Theme::timeNow * 4.2f - 0.55f) * 10.0f - 8.0f;
        bool sparklePose = ((int)(Theme::timeNow * 2.5f) % 2) == 1;

        std::string bodyKey = sparklePose
            ? "word_graduate_victory_ui_kit/mascot_layers/mascot_body_sparkle_aligned.png"
            : "word_graduate_victory_ui_kit/mascot_layers/mascot_body_cheer_aligned.png";
        UIAssets::drawCentered(bodyKey, { 270, 528 + bounceY }, 195, 195);
        UIAssets::drawCentered("word_graduate_victory_ui_kit/mascot_layers/graduation_cap_cropped.png",
                               { 270, 426 + bounceY + capLagY }, 135, 78,
                               std::sin(Theme::timeNow * 3.0f) * 4.5f);

        // Floating Coins & Magic XP Potion
        float floatL = std::sin(Theme::timeNow * 3.0f) * 6.0f;
        float floatR = std::cos(Theme::timeNow * 3.0f) * 6.0f;
        UIAssets::drawCentered("word_graduate_victory_ui_kit/rewards_and_stars/reward_coins_stack.png",
                               { 108, 545 + floatL }, 88, 88);
        Theme::text3D("+50", { 108, 598 + floatL }, 22, Color{ 253, 224, 71, 255 }, BLACK, 2.5f, BLACK, true, true);

        UIAssets::drawCentered("word_graduate_victory_ui_kit/rewards_and_stars/reward_magic_xp_potion.png",
                               { 432, 545 + floatR }, 88, 88);
        Theme::text3D("+25 XP", { 432, 598 + floatR }, 20, Color{ 103, 232, 249, 255 }, BLACK, 2.5f, BLACK, true, true);

        // 8. Open Word Flashcard Button ("بطاقة الكلمة التفصيلية")
        Rectangle cardBtnRec{ 130, 695, 280, 56 };
        UI::GradientRound(cardBtnRec, 18.0f, Color{ 245, 158, 11, 255 }, Color{ 217, 119, 6, 255 });
        UI::Rim(cardBtnRec, 18.0f, Color{ 120, 53, 15, 255 }, Color{ 254, 243, 199, 255 }, Color{ 180, 83, 9, 255 });
        Theme::textArabic("بطاقة الكلمة والمرادفات (WORD CARD)", { 270, 723 }, 21, WHITE, true, true, BLACK, 2.0f);

        // 9. Replay / Map Circle Button & Next Level CTA Button
        Rectangle replayRec{ 72, 772, 82, 82 };
        Rectangle nextRec{ 170, 770, 290, 82 };
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/btn_replay_circle.png", replayRec);
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/btn_next_level_cta.png", nextRec);

        Theme::drawParticles();

        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
```

---

### 3.4 الملف الرابع: `custom_engine/include/screen_home.hpp` *(ملف جديد — شاشة البداية BAC VOCAB QUEST مع الثلاثي المتحرك)*

انسخ هذا الكود بالكامل واحفظه في `custom_engine/include/screen_home.hpp`:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "word_card_modal.hpp"

#include <cmath>
#include <string>

class ScreenHome {
public:
    WordCardModal wordCardModal;
    int pendingAction = 0; // 0=none, 1=Play(Map), 2=BoosterShop, 3=DailyStreak, 4=GraduationAcademyMap

    void init() {
        wordCardModal.init();
    }

    int update(float dt) {
        pendingAction = 0;
        if (wordCardModal.active) {
            wordCardModal.update(dt);
            return -1;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();

            // Sound toggle button
            if (CheckCollisionPointRec(m, Rectangle{ 412, 16, 50, 50 })) {
                Theme::soundOn = !Theme::soundOn;
                gAudio.playClick();
                return -1;
            }
            // Main PLAY button
            if (CheckCollisionPointRec(m, Rectangle{ 115, 735, 310, 96 })) {
                gAudio.playFanfare();
                Theme::spawnCelebration({ 270, 780 }, 56);
                pendingAction = 1;
                return Nav::SCREEN_MAP;
            }
            // Bottom Dock: 4 Quick Feature Buttons
            // 1. Booster Shop
            if (CheckCollisionPointRec(m, Rectangle{ 18, 852, 118, 86 })) {
                gAudio.playPop();
                pendingAction = 2;
                return -1;
            }
            // 2. Daily Streak (1-7)
            if (CheckCollisionPointRec(m, Rectangle{ 146, 852, 118, 86 })) {
                gAudio.playPop();
                pendingAction = 3;
                return -1;
            }
            // 3. Graduation Academy Map (1-14)
            if (CheckCollisionPointRec(m, Rectangle{ 274, 852, 118, 86 })) {
                gAudio.playPop();
                pendingAction = 4;
                return -1;
            }
            // 4. Word Card ("INTEGRITY")
            if (CheckCollisionPointRec(m, Rectangle{ 402, 852, 120, 86 })) {
                WordData w;
                if (!gGameData.levels.empty()) {
                    w = gGameData.levels[0].word;
                }
                w.word = "INTEGRITY";
                w.translation_ar = "النزاهة / الاستقامة والأمانة";
                w.definition_en = "The quality of being honest and having strong moral principles.";
                w.synonym_en = "Honesty, Probity, Rectitude";
                w.frequency = "High Frequency - Ethics in Business";
                wordCardModal.show(w);
                return -1;
            }
            // Clicking Trio Mascots triggers celebration
            if (CheckCollisionPointRec(m, Rectangle{ 50, 410, 440, 290 })) {
                gAudio.playChime();
                Theme::spawnCelebration(m, 42);
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Living Sky Gradient Backdrop + Soft Clouds
        DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 224, 242, 254, 255 });
        DrawRectangleGradientV(0, h / 2, w, h / 2, Color{ 224, 242, 254, 255 }, Color{ 167, 243, 208, 255 });

        for (int i = 0; i < 5; ++i) {
            float cx = std::fmod(i * 145.0f + t * (12.0f + i * 3.0f), 680.0f) - 70.0f;
            float cy = 95.0f + (i % 3) * 110.0f;
            DrawCircle((int)cx, (int)cy, 38.0f, Color{ 255, 255, 255, 140 });
            DrawCircle((int)(cx + 28), (int)(cy + 6), 28.0f, Color{ 255, 255, 255, 140 });
            DrawCircle((int)(cx - 26), (int)(cy + 8), 26.0f, Color{ 255, 255, 255, 140 });
        }

        // 2. Floating Islands & Magic Books (from bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books)
        float isl1Y = std::sin(t * 2.1f) * 8.0f;
        float isl2Y = std::sin(t * 1.8f + 1.4f) * 9.0f;
        float isl3Y = std::sin(t * 2.4f + 2.8f) * 7.0f;

        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/island_01_observatory_telescope.png",
                               { 92, 250 + isl1Y }, 155, 155);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/island_02_classical_greek_temple.png",
                               { 448, 242 + isl2Y }, 155, 155);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/island_03_open_book_fairytale_castle.png",
                               { 270, 560 + isl3Y * 0.4f }, 480, 290);

        // Fluttering Magic Books
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/flying_book_01_cream.png",
                               { 68, 395 + std::sin(t * 3.8f) * 10.0f }, 76, 76, std::sin(t * 4.0f) * 8.0f);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/flying_book_02_purple.png",
                               { 472, 385 + std::cos(t * 3.4f) * 11.0f }, 76, 76, std::cos(t * 3.5f) * -8.0f);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/flying_book_03_red.png",
                               { 440, 128 + std::sin(t * 4.2f + 1.0f) * 8.0f }, 68, 68, std::sin(t * 3.2f) * 6.0f);

        // 3. Top HUD Bar (from bac_vocab_quest_ui_kit/01_hud_and_emblem_crest)
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/hud_avatar_lv1_badge.png",
                          Rectangle{ 14, 12, 165, 58 });
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/hud_heart_life_bar.png",
                          Rectangle{ 192, 16, 145, 50 });
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/btn_sound_speaker.png",
                          Rectangle{ 412, 16, 50, 50 });
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/btn_settings_gear.png",
                          Rectangle{ 472, 16, 50, 50 });

        // 4. Center Emblem Crest "BAC VOCAB QUEST" with Breathing Scale
        float crestScale = 1.0f + 0.03f * std::sin(t * 2.6f);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/emblem_bac_vocab_quest_crest.png",
                               { 270, 225 }, 350 * crestScale, 255 * crestScale);

        // 5. 3D Gold-Blue BAC Alphabet Banner ("BAC QUEST")
        UIAssets::drawWordTiles("04_bac_gold_blue_alphabet_a_to_z/bac_letter_",
                                "BACQUEST", { 270, 382 }, 390.0f, 44.0f, 3.0f);

        // 6. Animated Trio Mascots (Scholar Owl, Astronaut, Detective Fox)
        // Left: Scholar Owl (Body + Waving Wing)
        float owlBob = std::sin(t * 3.0f) * 6.0f;
        float wingRot = std::sin(t * 5.2f) * 16.0f;
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/scholar_owl_waving_wing.png",
                               { 158, 545 + owlBob }, 85, 85, wingRot);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/scholar_owl_body_clean.png",
                               { 122, 548 + owlBob }, 168, 185);

        // Right: Cheering Detective Fox
        float foxBob = std::abs(std::sin(t * 3.6f + 0.8f)) * -10.0f;
        float foxTilt = std::sin(t * 3.6f) * 4.5f;
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/cheering_fox_full_rgba.png",
                               { 415, 548 + foxBob }, 172, 188, foxTilt);

        // Center Hero: Celebrating Astronaut (Body + Articulated Left/Right Arms)
        float astroBob = std::sin(t * 2.8f + 0.4f) * 8.0f;
        float armSwing = std::sin(t * 4.8f) * 18.0f;
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/astronaut_left_arm.png",
                               { 212, 495 + astroBob }, 82, 82, -armSwing);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/astronaut_right_arm.png",
                               { 328, 495 + astroBob }, 82, 82, armSwing);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/astronaut_body_armless.png",
                               { 270, 520 + astroBob }, 190, 215);

        // 7. Emerald Play Tiles Preview ("PLAY") & Giant Emerald-Gold CTA Button
        float btnPulse = 1.0f + 0.04f * std::sin(t * 3.8f);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/btn_play_emerald_gold_with_text.png",
                               { 270, 782 }, 320 * btnPulse, 98 * btnPulse);

        // 8. Bottom Feature Dock (Shop, Daily Streak 1-7, Graduation Academy 1-14, Word Card INTEGRITY)
        struct DockBtn {
            Rectangle r;
            const char* iconKey;
            const char* labelEn;
            const char* labelAr;
            Color topCol;
            Color botCol;
        };
        DockBtn dock[4] = {
            { { 18, 852, 118, 86 }, "magical_alchemist_booster_shop_ui_kit/03_booster_icons_hd/icon_magic_lightbulb_hd.png",
              "SHOP", "المتجر", Color{ 124, 58, 237, 255 }, Color{ 76, 29, 149, 255 } },
            { { 146, 852, 118, 86 }, "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/icon_streak_fire_hd.png",
              "STREAK 1-7", "المثابرة", Color{ 234, 88, 12, 255 }, Color{ 154, 52, 18, 255 } },
            { { 274, 852, 118, 86 }, "image2_graduation_academy_map_assets/building_big_blue_castle.png",
              "ACADEMY", "الأكاديمية", Color{ 37, 99, 235, 255 }, Color{ 30, 58, 138, 255 } },
            { { 402, 852, 120, 86 }, "image1_integrity_word_card_ui_kit/icon_integrity_scale_hd.png",
              "WORD CARD", "بطاقة كلمة", Color{ 16, 185, 129, 255 }, Color{ 6, 95, 70, 255 } }
        };

        for (int i = 0; i < 4; ++i) {
            UI::DropShadow(dock[i].r, 18.0f, 4.0f, 0.35f, 2.0f);
            UI::GradientRound(dock[i].r, 18.0f, dock[i].topCol, dock[i].botCol);
            UI::Rim(dock[i].r, 18.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            UIAssets::drawCentered(dock[i].iconKey, { dock[i].r.x + dock[i].r.width * 0.5f, dock[i].r.y + 30.0f }, 44, 44);
            Theme::text3D(dock[i].labelEn, { dock[i].r.x + dock[i].r.width * 0.5f, dock[i].r.y + 66.0f },
                          14, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        Theme::drawParticles();
        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
```

---

### 3.5 الملف الخامس: `custom_engine/include/screen_shop.hpp` *(ملف جديد — متجر المعززات Booster Shop بخلفية حية)*

انسخ هذا الكود بالكامل واحفظه في `custom_engine/include/screen_shop.hpp`:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <cmath>
#include <string>

class ScreenShop {
public:
    int ownedBoosters[4] = { 3, 2, 1, 1 };
    int prices[4] = { 100, 150, 200, 250 };
    std::string statusMsg;
    float statusTimer = 0.0f;

    void init() {
        statusMsg.clear();
        statusTimer = 0.0f;
    }

    int update(float dt) {
        if (statusTimer > 0.0f) {
            statusTimer = std::max(0.0f, statusTimer - dt);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Close button (top right)
            if (CheckCollisionPointRec(m, Rectangle{ 462, 18, 62, 62 })) {
                gAudio.playClick();
                return Nav::SCREEN_HOME;
            }

            // 4 Buy Buttons inside 2x2 Booster Grid
            Rectangle buyRects[4] = {
                { 58, 462, 180, 56 },
                { 302, 462, 180, 56 },
                { 58, 792, 180, 56 },
                { 302, 792, 180, 56 }
            };

            for (int i = 0; i < 4; ++i) {
                if (CheckCollisionPointRec(m, buyRects[i])) {
                    if (Theme::coins >= prices[i]) {
                        Theme::coins -= prices[i];
                        ownedBoosters[i]++;
                        gAudio.playFanfare();
                        Theme::spawnCelebration({ buyRects[i].x + 90.0f, buyRects[i].y + 28.0f }, 36);
                        statusMsg = "تم شراء المعزز بنجاح!";
                        statusTimer = 2.2f;
                    } else {
                        gAudio.playPop();
                        statusMsg = "رصيد العملات غير كافٍ!";
                        statusTimer = 2.0f;
                    }
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Clean Alchemist Study Background
        if (!UIAssets::drawStretch("magical_alchemist_booster_shop_ui_kit/01_backgrounds/bg_alchemist_study_clean.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 28, 18, 56, 255 }, Color{ 14, 9, 32, 255 });
        }

        // 2. LIVING BACKGROUND FX (Flickering Candles, Glowing Potions, Twinkling Stars & Floating Magic Dust)
        // Note: All Shop UI Cards & Buttons remain 100% static as required!
        Vector2 candleSpots[4] = { { 72, 195 }, { 478, 198 }, { 48, 520 }, { 495, 515 } };
        for (int i = 0; i < 4; ++i) {
            float flicker = 0.75f + 0.25f * std::sin(t * (7.5f + i * 1.9f) + i)
                                  + 0.12f * std::cos(t * (13.0f + i * 2.3f));
            float rad = 36.0f * flicker;
            DrawCircleGradient((int)candleSpots[i].x, (int)candleSpots[i].y, rad,
                               Color{ 255, 200, 80, (unsigned char)(115 * flicker) },
                               Color{ 255, 140, 20, 0 });
        }

        // Rising golden dust particles in the background
        for (int i = 0; i < 28; ++i) {
            float px = std::fmod(i * 73.0f + std::sin(t * 1.2f + i) * 24.0f, (float)w);
            float py = h - std::fmod(t * (22.0f + (i % 5) * 6.0f) + i * 47.0f, (float)h);
            float alpha = 0.45f + 0.45f * std::sin(t * 3.5f + i * 1.7f);
            DrawCircleV({ px, py }, 2.2f + (i % 3), Color{ 253, 224, 71, (unsigned char)(180 * alpha) });
        }

        // 3. Static Shop Header, Coin Bar, and Close Button
        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/hud_coin_bar_blank.png",
                          Rectangle{ 16, 18, 175, 56 });
        Theme::text3D(std::to_string(Theme::coins), { 112, 46 }, 24, Color{ 253, 224, 71, 255 }, BLACK, 2.5f, BLACK, true, true);

        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/banner_booster_shop.png",
                          Rectangle{ 95, 78, 350, 92 });

        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/02_blank_ui_templates/btn_close_red_x.png",
                          Rectangle{ 462, 18, 62, 62 });

        // 4. 2x2 Static Booster Cards + Buy Buttons
        const char* cardKeys[4] = {
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_1_magic_lightbulb.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_2_detective_magnifier.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_3_rocket_eraser.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_4_master_dictionary.png"
        };
        const char* buyKeys[4] = {
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_100.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_150.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_200.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_250.png"
        };
        Rectangle cardRects[4] = {
            { 32, 188, 232, 268 },
            { 276, 188, 232, 268 },
            { 32, 518, 232, 268 },
            { 276, 518, 232, 268 }
        };
        Rectangle buyRects[4] = {
            { 58, 462, 180, 56 },
            { 302, 462, 180, 56 },
            { 58, 792, 180, 56 },
            { 302, 792, 180, 56 }
        };

        for (int i = 0; i < 4; ++i) {
            UIAssets::drawFit(cardKeys[i], cardRects[i]);
            UIAssets::drawFit(buyKeys[i], buyRects[i]);

            // Owned count badge in top-right corner of each card
            Vector2 badgeCenter{ cardRects[i].x + cardRects[i].width - 24.0f, cardRects[i].y + 22.0f };
            DrawCircleV(badgeCenter, 18.0f, Color{ 16, 185, 129, 255 });
            DrawCircleLines((int)badgeCenter.x, (int)badgeCenter.y, 18.0f, WHITE);
            Theme::text3D("x" + std::to_string(ownedBoosters[i]), badgeCenter, 16, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        if (statusTimer > 0.0f && !statusMsg.empty()) {
            Rectangle toast{ 90, 875, 360, 54 };
            UI::GradientRound(toast, 16.0f, Color{ 15, 23, 42, 235 }, Color{ 30, 58, 138, 235 });
            UI::Rim(toast, 16.0f, Color{ 250, 204, 21, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::textArabic(statusMsg, { 270, 902 }, 22, Color{ 254, 240, 138, 255 }, true, true, BLACK, 2.0f);
        }

        Theme::drawParticles();
    }
};
```

---

### 3.6 الملف السادس: `custom_engine/include/screen_streak.hpp` *(ملف جديد — مسار المثابرة اليومية Daily Streak 1–7)*

انسخ هذا الكود بالكامل واحفظه في `custom_engine/include/screen_streak.hpp`:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <cmath>
#include <string>

class ScreenStreak {
public:
    int currentStreakDay = 4;
    bool rewardClaimedToday = false;

    void init() {
        currentStreakDay = 4;
        rewardClaimedToday = false;
    }

    int update(float dt) {
        (void)dt;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Back / Close button
            if (CheckCollisionPointRec(m, Rectangle{ 16, 18, 56, 56 }) ||
                CheckCollisionPointRec(m, Rectangle{ 468, 18, 56, 56 })) {
                gAudio.playClick();
                return Nav::SCREEN_HOME;
            }
            // Claim Today's Reward CTA Button
            if (CheckCollisionPointRec(m, Rectangle{ 75, 848, 390, 82 })) {
                if (!rewardClaimedToday) {
                    rewardClaimedToday = true;
                    currentStreakDay = std::min(7, currentStreakDay + 1);
                    Theme::coins += 120;
                    gAudio.playFanfare();
                    Theme::spawnCelebration({ 270, 880 }, 72);
                } else {
                    gAudio.playPop();
                }
            }
            // Clicking Grand Chest triggers vibration burst
            if (CheckCollisionPointRec(m, Rectangle{ 260, 235, 220, 175 })) {
                gAudio.playChime();
                Theme::spawnCelebration({ 370, 315 }, 48);
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Clean Forest & Roadmap Base (without baked owl or chest so they animate cleanly)
        if (!UIAssets::drawStretch("daily_streak_roadmap_ui_kit/01_screens_and_boards/daily_streak_clean_base_no_owl_no_chest.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            UIAssets::drawStretch("daily_streak_roadmap_ui_kit/01_screens_and_boards/roadmap_board_clean_1_to_7.png",
                                  Rectangle{ 0, 0, (float)w, (float)h });
        }

        // 2. Top Header Banner, 12-Days Streak Badge & 14-Days Exam Countdown Banner
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/header_daily_streak_title.png",
                          Rectangle{ 75, 16, 390, 85 });
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/badge_streak_12_days.png",
                          Rectangle{ 95, 102, 350, 72 });
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/banner_14_days_exam.png",
                          Rectangle{ 58, 178, 424, 58 });

        // Close Button (top right)
        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/02_blank_ui_templates/btn_close_red_x.png",
                          Rectangle{ 470, 20, 54, 54 });

        // 3. Corrected Sequential Day 1 -> Day 7 Stumps along the Winding Path
        struct DayNode {
            const char* key;
            Vector2 pos;
            Vector2 size;
            const char* label;
        };
        DayNode days[7] = {
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_1.png", { 118, 725 }, { 108, 95 }, "DAY 1" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_2.png", { 265, 718 }, { 108, 95 }, "DAY 2" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_3.png", { 412, 682 }, { 108, 95 }, "DAY 3" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_4.png", { 378, 565 }, { 108, 95 }, "DAY 4" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_5.png", { 228, 548 }, { 108, 95 }, "DAY 5" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_6.png", { 225, 435 }, { 108, 95 }, "DAY 6" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_7_pedestal.png", { 372, 385 }, { 165, 110 }, "DAY 7" }
        };

        for (int i = 0; i < 7; ++i) {
            float pulse = (i + 1 == currentStreakDay) ? (1.0f + 0.06f * std::sin(t * 4.5f)) : 1.0f;
            UIAssets::drawCentered(days[i].key, days[i].pos, days[i].size.x * pulse, days[i].size.y * pulse);

            Color lblCol = (i + 1 <= currentStreakDay) ? Color{ 253, 224, 71, 255 } : WHITE;
            Theme::text3D(days[i].label, { days[i].pos.x, days[i].pos.y + 38.0f },
                          16, lblCol, Color{ 66, 32, 6, 255 }, 2.5f, BLACK, true, true);
        }

        // 4. Vibrating Grand BAC Mega Bonus Chest on Day 7 Pedestal
        float shakePhase = std::sin(t * 28.0f) * std::max(0.0f, std::sin(t * 2.6f));
        float chestRot = shakePhase * 6.5f;
        float chestScale = 1.0f + std::abs(shakePhase) * 0.05f;
        float chestJump = std::abs(shakePhase) * -7.0f;

        DrawCircleGradient(372, 318, 95.0f, Color{ 253, 224, 71, 95 }, Color{ 253, 224, 71, 0 });
        UIAssets::drawCentered("daily_streak_roadmap_ui_kit/02_characters_and_chest/prop_bac_mega_bonus_chest_hd.png",
                               { 372, 318 + chestJump }, 185 * chestScale, 155 * chestScale, chestRot);

        // 5. Animated Professor Owl Mascot (Pose A <-> Pose B + Bobbing & Tilt)
        float owlBob = std::sin(t * 3.2f) * 7.0f;
        float owlTilt = std::sin(t * 2.1f) * 3.5f;
        bool poseB = ((int)(t * 2.4f) % 2) == 1;
        const char* owlKey = poseB
            ? "daily_streak_roadmap_ui_kit/02_characters_and_chest/char_professor_owl_pose_b.png"
            : "daily_streak_roadmap_ui_kit/02_characters_and_chest/char_professor_owl_pose_a.png";
        UIAssets::drawCentered(owlKey, { 112, 385 + owlBob }, 195, 225, owlTilt);

        // 6. Claim Today's Reward CTA Button
        Rectangle claimRec{ 75, 848, 390, 82 };
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/btn_claim_todays_reward.png", claimRec);

        Theme::drawParticles();
    }
};
```

---

### 3.7 الملف السابع: `custom_engine/include/screen_maps.hpp` *(تحديث كامل — خريطة أكاديمية التخرج 1–14 + خرائط الوحدات)*

استبدل محتوى الملف [`custom_engine/include/screen_maps.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_maps.hpp) بالكود الكامل التالي:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

class ScreenMaps {
public:
    struct NodeDef {
        float x;
        float y;
        int number;
    };

    struct UnitNodes {
        std::vector<NodeDef> nodes;
        float signX = 0.0f, signY = 0.0f, signW = 0.0f, signH = 0.0f;
        bool hasSign = false;
    };

    int unit = 0; // 0=Ethics, 1=Safety First, 2=Astronomy/Space, 3=Graduation Academy (Scrolls 1-14)
    float scrollY = 0.0f;
    float targetScrollY = 0.0f;
    int selectedLevel = -1;
    int pendingNavScreen = -1; // For switching to Home, Shop, Streak
    std::vector<UnitNodes> units;

    // Graduation Academy Boy march state along scrolls 1 -> 14
    int boyTargetScroll = 4;
    float boyProgress = 3.0f; // 0-based index along the 14 scrolls
    bool boyMarching = false;

    void init() {
        units.resize(4);

        // Unit 0: Ethics Island (10 nodes)
        units[0].nodes = {
            { 270, 760, 1 }, { 215, 712, 2 }, { 195, 655, 3 }, { 248, 608, 4 }, { 315, 568, 5 },
            { 330, 512, 6 }, { 268, 472, 7 }, { 212, 430, 8 }, { 248, 382, 9 }, { 275, 330, 10 }
        };
        units[0].hasSign = true;
        units[0].signX = 160; units[0].signY = 820; units[0].signW = 220; units[0].signH = 110;

        // Unit 1: Safety First / Food Pyramid Island (10 nodes)
        units[1].nodes = {
            { 268, 855, 1 }, { 218, 795, 2 }, { 292, 738, 3 }, { 235, 675, 4 }, { 295, 612, 5 },
            { 230, 548, 6 }, { 285, 482, 7 }, { 235, 415, 8 }, { 280, 342, 9 }, { 268, 260, 10 }
        };

        // Unit 2: Space / Astronomy Observatory Island (10 nodes)
        units[2].nodes = {
            { 270, 825, 1 }, { 340, 760, 2 }, { 225, 705, 3 }, { 188, 635, 4 }, { 285, 575, 5 },
            { 355, 505, 6 }, { 255, 445, 7 }, { 195, 385, 8 }, { 255, 325, 9 }, { 235, 265, 10 }
        };

        // Unit 3: Graduation Academy Map (14 Numbered Scrolls + Telegrams reaching the Grand Blue Castle)
        units[3].nodes = {
            { 165, 845, 1 },
            { 268, 825, 2 },
            { 372, 792, 3 },
            { 312, 738, 4 },
            { 195, 702, 5 },
            { 152, 642, 6 },
            { 248, 598, 7 },
            { 358, 560, 8 },
            { 382, 495, 9 },
            { 282, 452, 10 }, // Entering the Giant Open Book Bridge
            { 225, 392, 11 }, // Middle of the Giant Open Book Bridge
            { 278, 338, 12 }, // Upper Book Bridge
            { 335, 282, 13 }, // Castle Courtyard
            { 285, 222, 14 }  // Grand Blue Castle Gate!
        };
    }

    void setUnit(int u) {
        unit = std::clamp(u, 0, 3);
        selectedLevel = -1;
    }

    int update(float dt) {
        selectedLevel = -1;
        pendingNavScreen = -1;

        // Animate Boy walking along the 14 scrolls toward the Blue Castle
        if (boyMarching) {
            boyProgress += dt * 2.2f;
            if (boyProgress >= 13.0f) {
                boyProgress = 13.0f;
                boyMarching = false;
                gAudio.playFanfare();
                Theme::spawnCelebration({ 285, 200 }, 64);
            }
        } else {
            float target = (float)std::clamp(boyTargetScroll - 1, 0, 13);
            if (std::abs(boyProgress - target) > 0.02f) {
                float dir = (target > boyProgress) ? 1.0f : -1.0f;
                boyProgress += dir * dt * 3.0f;
                if ((dir > 0 && boyProgress > target) || (dir < 0 && boyProgress < target)) {
                    boyProgress = target;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();

            // Top Unit Selector Tabs (0=Ethics, 1=Safety, 2=Space, 3=Graduation Academy)
            for (int u = 0; u < 4; ++u) {
                Rectangle tabRec{ 12.0f + u * 130.0f, 72.0f, 124.0f, 38.0f };
                if (CheckCollisionPointRec(m, tabRec)) {
                    setUnit(u);
                    gAudio.playClick();
                    return -1;
                }
            }

            // Bottom Navigation Buttons: Home, Shop, Streak, March Boy (on Academy Map)
            if (CheckCollisionPointRec(m, Rectangle{ 14, 892, 118, 56 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_HOME;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 142, 892, 118, 56 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_SHOP;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 270, 892, 118, 56 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_STREAK;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 398, 892, 128, 56 })) {
                if (unit == 3) {
                    boyProgress = 0.0f;
                    boyTargetScroll = 14;
                    boyMarching = true;
                    gAudio.playFanfare();
                } else {
                    setUnit((unit + 1) % 4);
                    gAudio.playClick();
                }
                return -1;
            }

            // Node / Scroll clicks
            const auto& nodes = units[unit].nodes;
            for (size_t i = 0; i < nodes.size(); ++i) {
                float hitRadius = (unit == 3) ? 34.0f : 28.0f;
                if (CheckCollisionPointCircle(m, { nodes[i].x, nodes[i].y }, hitRadius)) {
                    gAudio.playPop();
                    if (unit == 3) {
                        boyTargetScroll = (int)i + 1;
                        boyMarching = false;
                    }
                    int totalLvls = std::max(1, gGameData.levelCount());
                    int startIdx = (unit < 3) ? gGameData.unitStart(unit) : 0;
                    selectedLevel = (startIdx + (int)i) % totalLvls;
                    return selectedLevel;
                }
            }
        }
        return -1;
    }

    void drawGraduationAcademyMap(int w, int h) {
        float t = Theme::timeNow;

        // 1. Clean Academy Map Backdrop
        if (!UIAssets::drawStretch("image2_graduation_academy_map_assets/map_background_clean.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 186, 230, 253, 255 });
        }

        // 2. Giant Open Book Bridge, Clock Tower, and Grand Blue Castle
        UIAssets::drawCentered("image2_graduation_academy_map_assets/bridge_giant_open_book.png",
                               { 255, 405 }, 255, 165);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/building_clock_tower.png",
                               { 102, 235 }, 145, 225);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/building_big_blue_castle.png",
                               { 312, 165 }, 290, 245);

        // 3. Waving Castle Flags atop the Grand Blue Castle
        bool flagPose2 = ((int)(t * 4.5f) % 2) == 1;
        float flagTilt = std::sin(t * 5.0f) * 6.0f;
        const char* flagKey = flagPose2
            ? "image2_graduation_academy_map_assets/prop_castle_flag_pose2.png"
            : "image2_graduation_academy_map_assets/prop_castle_flag_pose1.png";
        UIAssets::drawCentered(flagKey, { 248, 68 }, 54, 54, flagTilt);
        UIAssets::drawCentered(flagKey, { 372, 68 }, 54, 54, -flagTilt);

        // 4. Spinning Globe, Chalkboard, ABC Blocks & Flying Magic Books
        float globeTilt = std::sin(t * 2.8f) * 12.0f;
        UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_globe_full.png",
                               { 442, 650 }, 118, 135, globeTilt);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_chalkboard.png",
                               { 95, 515 }, 135, 120);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/island_abc_blocks.png",
                               { 88, 795 }, 135, 115);

        bool bookPose2 = ((int)(t * 5.2f) % 2) == 1;
        const char* bookKey = bookPose2
            ? "image2_graduation_academy_map_assets/prop_flying_book_pose2.png"
            : "image2_graduation_academy_map_assets/prop_flying_book_pose1.png";
        UIAssets::drawCentered(bookKey, { 125, 355 + std::sin(t * 4.0f) * 9.0f }, 78, 68, std::sin(t * 3.5f) * 8.0f);
        UIAssets::drawCentered(bookKey, { 435, 365 + std::cos(t * 3.8f) * 9.0f }, 74, 64, std::cos(t * 3.2f) * -8.0f);

        // 5. Golden Path Connecting all 14 Scrolls & Telegrams up to the Grand Blue Castle
        const auto& nodes = units[3].nodes;
        for (size_t i = 0; i + 1 < nodes.size(); ++i) {
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       8.0f, Color{ 251, 191, 36, 175 });
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       3.5f, Color{ 254, 249, 195, 235 });
        }

        // Draw all 14 Numbered Scrolls (scroll_step_01.png .. scroll_step_14.png)
        for (size_t i = 0; i < nodes.size(); ++i) {
            char scrollKey[128];
            std::snprintf(scrollKey, sizeof(scrollKey),
                          "image2_graduation_academy_map_assets/numbered_scrolls_1_to_14/scroll_step_%02d.png",
                          (int)i + 1);
            float pulse = ((int)i == (int)std::round(boyProgress)) ? (1.0f + 0.08f * std::sin(t * 5.0f)) : 1.0f;
            if (!UIAssets::drawCentered(scrollKey, { nodes[i].x, nodes[i].y }, 86 * pulse, 64 * pulse)) {
                UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_scroll_and_telegram_step.png",
                                       { nodes[i].x, nodes[i].y }, 86 * pulse, 64 * pulse);
                Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y - 4.0f },
                              18, Color{ 120, 53, 15, 255 }, WHITE, 2.0f, BLANK, true, true);
            }
        }

        // 6. Animated Owl Flapping Above the Academy
        bool owlFlap = ((int)(t * 4.8f) % 2) == 1;
        const char* owlKey = owlFlap
            ? "image2_graduation_academy_map_assets/char_owl_pose2_flapping.png"
            : "image2_graduation_academy_map_assets/char_owl_pose1_idle.png";
        UIAssets::drawCentered(owlKey, { 418, 225 + std::sin(t * 3.6f) * 10.0f }, 96, 96);

        // 7. Animated Boy Student Moving Across Scrolls (1 -> 14) to the Blue Castle
        int idxA = std::clamp((int)std::floor(boyProgress), 0, 13);
        int idxB = std::clamp(idxA + 1, 0, 13);
        float frac = boyProgress - (float)idxA;
        Vector2 boyPos{
            nodes[idxA].x + (nodes[idxB].x - nodes[idxA].x) * frac,
            nodes[idxA].y + (nodes[idxB].y - nodes[idxA].y) * frac - 34.0f
        };
        float hopY = std::abs(std::sin(t * 5.5f)) * -12.0f;
        bool boyJump = ((int)(t * 4.0f) % 2) == 1 || boyMarching;
        const char* boyKey = boyJump
            ? "image2_graduation_academy_map_assets/char_boy_pose2_jumping.png"
            : "image2_graduation_academy_map_assets/char_boy_pose1_cheering.png";
        UIAssets::drawCentered(boyKey, { boyPos.x, boyPos.y + hopY }, 105, 118);

        // 8. Graduation Academy Banner
        UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_graduation_academy_banner.png",
                               { 270, 132 }, 320, 66);
    }

    void drawSafetyFoodIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawStretch("assets/scenery/clean_safety.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 30, 136, 229, 255 }, Color{ 2, 119, 189, 255 });
        }

        // Animated Chef & Dog Inspector from food_pyramid_island_ui_kit
        float chefBob = std::sin(t * 3.2f) * 5.0f;
        UIAssets::drawCentered("food_pyramid_island_ui_kit/02_characters_animated/chef_mascot_waving.png",
                               { 355, 455 + chefBob }, 125, 135);
        UIAssets::drawCentered("food_pyramid_island_ui_kit/02_characters_animated/dog_inspector_full.png",
                               { 420, 645 + std::cos(t * 3.5f) * 4.0f }, 105, 105);

        const auto& nodes = units[1].nodes;
        for (size_t i = 0; i < nodes.size(); ++i) {
            char key[128];
            std::snprintf(key, sizeof(key),
                          "food_pyramid_island_ui_kit/04_level_buttons_1_to_10/level_%02d_button.png",
                          (int)i + 1);
            if (!UIAssets::drawCentered(key, { nodes[i].x, nodes[i].y }, 68, 68)) {
                DrawCircleV({ nodes[i].x, nodes[i].y }, 26.0f, Color{ 245, 158, 11, 255 });
                Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y }, 20, WHITE, BLACK, 2.0f, BLACK, true, true);
            }
        }
    }

    void drawCosmicSpaceIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawStretch("assets/scenery/clean_space.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 11, 15, 41, 255 }, Color{ 46, 16, 101, 255 });
        }

        // Animated Astronaut, Rocket & Satellite from cosmic_space_observatory_ui_kit
        UIAssets::drawCentered("cosmic_space_observatory_ui_kit/03_characters_and_vehicles/astronaut_with_star_flag.png",
                               { 295, 175 + std::sin(t * 2.8f) * 7.0f }, 120, 120);
        UIAssets::drawCentered("cosmic_space_observatory_ui_kit/03_characters_and_vehicles/space_satellite_solar_wings.png",
                               { 435, 215 + std::cos(t * 2.4f) * 9.0f }, 115, 95, std::sin(t * 1.8f) * 6.0f);

        const auto& nodes = units[2].nodes;
        for (size_t i = 0; i < nodes.size(); ++i) {
            char key[128];
            std::snprintf(key, sizeof(key),
                          "cosmic_space_observatory_ui_kit/04_level_nodes_01_to_10/node_%02d_cyan.png",
                          (int)i + 1);
            if (!UIAssets::drawCentered(key, { nodes[i].x, nodes[i].y }, 68, 68)) {
                DrawCircleV({ nodes[i].x, nodes[i].y }, 26.0f, Color{ 6, 182, 212, 255 });
                Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y }, 20, WHITE, BLACK, 2.0f, BLACK, true, true);
            }
        }
    }

    void drawEthicsIsland(int w, int h) {
        if (!UIAssets::drawStretch("assets/scenery/clean_ethics.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 174, 226, 255, 255 }, Color{ 243, 232, 255, 255 });
        }
        const auto& nodes = units[0].nodes;
        for (size_t i = 0; i < nodes.size(); ++i) {
            Color c = (i == 0) ? Color{ 14, 165, 233, 255 } : Color{ 148, 163, 184, 255 };
            DrawCircleV({ nodes[i].x, nodes[i].y + 4.0f }, 27.0f, Color{ 15, 23, 42, 140 });
            DrawCircleV({ nodes[i].x, nodes[i].y }, 26.0f, c);
            DrawCircleLines((int)nodes[i].x, (int)nodes[i].y, 26.0f, WHITE);
            Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y }, 20, WHITE, BLACK, 2.0f, BLACK, true, true);
        }
    }

    void draw(int w, int h) {
        if (unit == 0) drawEthicsIsland(w, h);
        else if (unit == 1) drawSafetyFoodIsland(w, h);
        else if (unit == 2) drawCosmicSpaceIsland(w, h);
        else drawGraduationAcademyMap(w, h);

        // Top HUD Header Bar
        UIAssets::drawFit("cosmic_space_observatory_ui_kit/01_hud_bars_and_buttons/hud_top_star_progress_bar.png",
                          Rectangle{ 14, 10, 260, 52 });
        UIAssets::drawFit("cosmic_space_observatory_ui_kit/01_hud_bars_and_buttons/hud_top_coin_counter_pill.png",
                          Rectangle{ 310, 12, 145, 48 });
        Theme::text3D(std::to_string(Theme::coins), { 398, 36 }, 20, Color{ 253, 224, 71, 255 }, BLACK, 2.0f, BLACK, true, true);

        // 4 Unit Tabs (Ethics, Safety, Space, Academy 1-14)
        const char* tabLabels[4] = { "1.ETHICS", "2.SAFETY", "3.SPACE", "4.ACADEMY" };
        for (int u = 0; u < 4; ++u) {
            Rectangle tabRec{ 12.0f + u * 130.0f, 72.0f, 124.0f, 38.0f };
            Color topC = (unit == u) ? Color{ 251, 191, 36, 255 } : Color{ 30, 41, 59, 215 };
            Color botC = (unit == u) ? Color{ 217, 119, 6, 255 } : Color{ 15, 23, 42, 215 };
            UI::GradientRound(tabRec, 14.0f, topC, botC);
            UI::Rim(tabRec, 14.0f, WHITE, WHITE, Color{ 100, 116, 139, 255 });
            Theme::text3D(tabLabels[u], { tabRec.x + 62.0f, tabRec.y + 19.0f },
                          15, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        // Bottom Navigation Bar (Home, Shop, Streak 1-7, March/Next)
        const char* botLabels[4] = { "HOME", "SHOP", "STREAK", (unit == 3) ? "GO 1->14" : "NEXT MAP" };
        Color botCols[4] = {
            Color{ 16, 185, 129, 255 },
            Color{ 124, 58, 237, 255 },
            Color{ 234, 88, 12, 255 },
            Color{ 37, 99, 235, 255 }
        };
        for (int i = 0; i < 4; ++i) {
            Rectangle r{ 14.0f + i * 128.0f, 892.0f, 120.0f, 54.0f };
            UI::DropShadow(r, 16.0f, 4.0f, 0.35f, 2.0f);
            UI::GradientRound(r, 16.0f, botCols[i], Color{ 15, 23, 42, 255 });
            UI::Rim(r, 16.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::text3D(botLabels[i], { r.x + 60.0f, r.y + 27.0f }, 16, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        Theme::drawParticles();
    }
};
```

---

### 3.8 الملف الثامن: `custom_engine/include/screen_crossword.hpp` *(تحديث كامل — الكلمات المتقاطعة مع الثعلب المتحرك)*

استبدل محتوى الملف [`custom_engine/include/screen_crossword.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_crossword.hpp) بالكود الكامل التالي:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "victory_modal.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>

class ScreenCrossword {
public:
    struct TrayOrb {
        std::string letter;
        Vector2 pos{};
        Color color{};
        Color shadow{};
        bool used = false;
        bool emptySocket = false;
    };

    CrosswordLayout layout;
    VictoryModal victoryModal;
    std::vector<TrayOrb> trayOrbs;
    std::string currentGuess;
    int levelIndex = 0;
    int hintsRemaining = 3;
    bool levelCompleted = false;

    void init() {
        victoryModal.init();
        setLevel(0);
    }

    void setLevel(int idx) {
        if (gGameData.levels.empty()) return;
        levelIndex = std::clamp(idx, 0, (int)gGameData.levels.size() - 1);
        const auto& lvl = gGameData.levels[levelIndex];
        int u = std::clamp(lvl.unit, 0, std::max(0, (int)gGameData.units.size() - 1));
        if (!gGameData.units.empty()) {
            layout.build(lvl.word, gGameData.units[u], 2);
        }
        currentGuess.clear();
        levelCompleted = false;
        victoryModal.hide();
        buildTrayOrbs();
    }

    void buildTrayOrbs() {
        trayOrbs.clear();
        if (gGameData.levels.empty()) return;
        std::string w = gGameData.levels[levelIndex].word.word;
        const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        std::vector<char> pool;
        for (char c : w) {
            if (std::isalpha((unsigned char)c)) pool.push_back((char)std::toupper((unsigned char)c));
        }
        while (pool.size() < 14) {
            char extra = alphabet[(pool.size() * 7 + levelIndex * 3) % 26];
            pool.push_back(extra);
        }

        for (int i = 0; i < 14; ++i) {
            int row = i / 7;
            int col = i % 7;
            TrayOrb orb;
            orb.letter = std::string(1, pool[i]);
            orb.pos = { 62.0f + col * 69.0f, 862.0f + row * 52.0f };
            orb.used = false;
            orb.emptySocket = false;
            trayOrbs.push_back(orb);
        }
    }

    void revealHint() {
        if (gGameData.levels.empty()) return;
        const std::string& target = gGameData.levels[levelIndex].word.word;
        if (currentGuess.size() < target.size()) {
            char nextChar = (char)std::toupper((unsigned char)target[currentGuess.size()]);
            currentGuess.push_back(nextChar);
            for (auto& orb : trayOrbs) {
                if (!orb.used && !orb.letter.empty() && orb.letter[0] == nextChar) {
                    orb.used = true;
                    break;
                }
            }
            gAudio.playPop();
            if (currentGuess == target) {
                levelCompleted = true;
                gGameData.completeLevel(levelIndex, 3);
                victoryModal.show(gGameData.levels[levelIndex].word);
            }
        }
    }

    int update(float dt) {
        if (victoryModal.active) {
            victoryModal.update(dt);
            if (victoryModal.nextClicked) {
                setLevel((levelIndex + 1) % std::max(1, gGameData.levelCount()));
                return -1;
            }
            if (victoryModal.mapClicked) {
                victoryModal.hide();
                return Nav::SCREEN_MAP;
            }
            return -1;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Back / Settings to Map
            if (CheckCollisionPointRec(m, Rectangle{ 340, 12, 52, 52 })) {
                gAudio.playClick();
                return Nav::SCREEN_MAP;
            }
            // Hint button below Detective Fox or Reveal button
            if (CheckCollisionPointCircle(m, { 68, 618 }, 36.0f) ||
                CheckCollisionPointCircle(m, { 478, 782 }, 36.0f)) {
                revealHint();
                return -1;
            }
            // Tray Orbs
            for (auto& orb : trayOrbs) {
                if (!orb.used && CheckCollisionPointCircle(m, orb.pos, 26.0f)) {
                    orb.used = true;
                    currentGuess += orb.letter;
                    gAudio.playPop();

                    const std::string& target = gGameData.levels[levelIndex].word.word;
                    if (currentGuess == target) {
                        levelCompleted = true;
                        gGameData.completeLevel(levelIndex, 3);
                        victoryModal.show(gGameData.levels[levelIndex].word);
                    } else if (currentGuess.size() >= target.size()) {
                        currentGuess.clear();
                        for (auto& o : trayOrbs) o.used = false;
                    }
                    break;
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Isometric Light-Blue Backdrop + Props from ethics_quest_crossword_ui_kit
        DrawRectangleGradientV(0, 0, w, h, Color{ 188, 227, 250, 255 }, Color{ 221, 242, 253, 255 });
        UIAssets::drawCentered("ethics_quest_crossword_ui_kit/05_containers_and_background_props/bg_prop_city_buildings_isometric.png",
                               { 88, 125 }, 145, 110);
        UIAssets::drawCentered("ethics_quest_crossword_ui_kit/05_containers_and_background_props/bg_prop_scales_of_justice.png",
                               { 270, 118 }, 95, 95);
        UIAssets::drawCentered("ethics_quest_crossword_ui_kit/05_containers_and_background_props/bg_prop_handshake_agreement.png",
                               { 452, 125 }, 125, 95);
        UIAssets::drawCentered("ethics_quest_crossword_ui_kit/05_containers_and_background_props/bg_prop_approved_contracts.png",
                               { 475, 585 }, 105, 105);

        // 2. Top Header & Currency Counters
        UIAssets::drawFit("ethics_quest_crossword_ui_kit/01_hud_and_action_buttons/hud_header_level_45_ethics_quest.png",
                          Rectangle{ 12, 10, 320, 82 });
        UIAssets::drawFit("ethics_quest_crossword_ui_kit/01_hud_and_action_buttons/btn_settings_wood_gear.png",
                          Rectangle{ 340, 14, 54, 54 });
        UIAssets::drawFit("ethics_quest_crossword_ui_kit/01_hud_and_action_buttons/hud_counter_450_coins.png",
                          Rectangle{ 404, 10, 124, 42 });
        UIAssets::drawFit("ethics_quest_crossword_ui_kit/01_hud_and_action_buttons/hud_counter_12_gems.png",
                          Rectangle{ 404, 54, 124, 42 });

        // 3. Dynamic 3D Wooden Crossword Grid using wood_tile_A..Z & wood_slot_recessed_empty.png
        std::string targetWord = gGameData.levels.empty() ? "BRIBERY" : gGameData.levels[levelIndex].word.word;
        int n = (int)targetWord.size();
        float tileSz = std::min(56.0f, 410.0f / std::max(1, n));
        float totalW = n * (tileSz + 4.0f) - 4.0f;
        float startX = 270.0f - totalW * 0.5f;
        float rowY = 335.0f;

        // Companion vertical words from CrosswordLayout
        for (int r = -2; r <= 2; ++r) {
            if (r == 0) continue;
            Rectangle compRec{ startX + (n / 2) * (tileSz + 4.0f), rowY + r * (tileSz + 4.0f), tileSz, tileSz };
            char compChar = (char)('A' + ((std::abs(r) * 5 + levelIndex) % 26));
            std::string key = std::string("03_wooden_crossword_tiles_a_to_z/wood_tile_") + compChar + ".png";
            UIAssets::drawFit(key, compRec);
        }

        // Golden Highlight Row around active target word
        Rectangle glowRec{ startX - 10.0f, rowY - 10.0f, totalW + 20.0f, tileSz + 20.0f };
        DrawRectangleRoundedLinesEx(glowRec, 0.22f, 8, 4.0f, Color{ 253, 224, 71, 235 });

        for (int i = 0; i < n; ++i) {
            Rectangle cellRec{ startX + i * (tileSz + 4.0f), rowY, tileSz, tileSz };
            char shown = '\0';
            if (i < (int)currentGuess.size()) {
                shown = currentGuess[i];
            } else if (i == 0 || i == n - 1) {
                shown = (char)std::toupper((unsigned char)targetWord[i]);
            }
            if (shown != '\0') {
                std::string key = std::string("03_wooden_crossword_tiles_a_to_z/wood_tile_") + shown + ".png";
                UIAssets::drawFit(key, cellRec);
            } else {
                UIAssets::drawFit("03_wooden_crossword_tiles_a_to_z/wood_slot_recessed_empty.png", cellRec);
            }
        }

        // 4. Animated Detective Fox (60-frame master sequence or layered rig) + Hint Button
        Rectangle foxRec{ 8, 425, 165, 175 };
        if (!UIAssets::drawFrameSequence("06_frames_master_fox/frame_", 60, t, 24.0f, foxRec)) {
            UIAssets::drawFit("02_detective_fox_animations_and_layers/fox_base_body_clean.png", foxRec);
        }
        UIAssets::drawCentered("ethics_quest_crossword_ui_kit/01_hud_and_action_buttons/btn_hint_magnifying_glass.png",
                               { 68, 618 }, 84, 84);

        // 5. Frosted Glass Definition Panel & Reveal Lightbulb Button
        Rectangle defRec{ 22, 672, 496, 138 };
        if (!UIAssets::drawStretch("ethics_quest_crossword_ui_kit/05_containers_and_background_props/panel_definition_frosted_glass.png", defRec)) {
            UI::GlassPanel(defRec, 22.0f, Color{ 120, 155, 180, 190 }, WHITE);
        }
        std::string defText = gGameData.levels.empty()
            ? "Giving or receiving money illegally for influence"
            : gGameData.levels[levelIndex].word.definition_en;
        Theme::text3D("DEFINITION:", { 46, 690 }, 20, Color{ 250, 204, 21, 255 }, Color{ 66, 32, 6, 255 }, 2.5f, BLACK, false, false);
        Theme::text3D(defText, { 46, 724 }, 17, WHITE, Color{ 30, 41, 59, 255 }, 2.0f, BLACK, false, false);

        UIAssets::drawCentered("ethics_quest_crossword_ui_kit/01_hud_and_action_buttons/btn_reveal_lightbulb.png",
                               { 478, 782 }, 82, 82);

        // 6. Bottom Wooden Keyboard Rack & Circular Glossy Letter Tokens (A-Z)
        Rectangle trayRec{ 12, 825, 516, 124 };
        UIAssets::drawStretch("ethics_quest_crossword_ui_kit/05_containers_and_background_props/tray_bottom_wooden_keyboard_rack.png", trayRec);

        const char* colorSuffixes[8] = {
            "_yellow.png", "_green.png", "_orange.png", "_red.png",
            "_teal.png", "_blue.png", "_purple.png", "_silver.png"
        };
        for (size_t i = 0; i < trayOrbs.size(); ++i) {
            const auto& orb = trayOrbs[i];
            DrawCircleV(orb.pos, 23.0f, Color{ 74, 37, 17, 220 }); // Recessed socket
            if (!orb.used && !orb.letter.empty()) {
                char c = (char)std::toupper((unsigned char)orb.letter[0]);
                int colorIdx = (c - 'A') % 8;
                std::string tokKey = std::string("04_circular_tray_tokens_a_to_z/token_") + c + colorSuffixes[colorIdx];
                if (!UIAssets::drawCentered(tokKey, orb.pos, 52, 52)) {
                    DrawCircleV(orb.pos, 23.0f, Color{ 245, 158, 11, 255 });
                    Theme::text3D(orb.letter, orb.pos, 22, WHITE, BLACK, 2.0f, BLACK, true, true);
                }
            }
        }

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
```

---

### 3.9 الملف التاسع: `custom_engine/include/screen_image_guess.hpp` *(تحديث كامل — تخمين الصورة مع الروبوت المتحرك)*

استبدل محتوى الملف [`custom_engine/include/screen_image_guess.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_image_guess.hpp) بالكود الكامل التالي:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "victory_modal.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>

class ScreenImageGuess {
public:
    VictoryModal victoryModal;
    std::string targetWord = "ECLIPSE";
    std::string currentGuess;
    std::vector<char> keys;
    std::vector<bool> keyUsed;
    int currentLevelIdx = 0;

    void init() {
        victoryModal.init();
        setLevel(0);
    }

    void setLevel(int index) {
        if (gGameData.levels.empty()) return;
        currentLevelIdx = std::clamp(index, 0, (int)gGameData.levels.size() - 1);
        targetWord = gGameData.levels[currentLevelIdx].word.word;
        currentGuess.clear();
        victoryModal.hide();

        keys.clear();
        keyUsed.clear();
        for (char c : targetWord) {
            if (std::isalpha((unsigned char)c)) keys.push_back((char)std::toupper((unsigned char)c));
        }
        const std::string extra = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        while (keys.size() < 14) {
            keys.push_back(extra[(keys.size() * 5 + currentLevelIdx * 3) % 26]);
        }
        keyUsed.assign(keys.size(), false);
    }

    void revealHint() {
        if (currentGuess.size() < targetWord.size()) {
            char c = (char)std::toupper((unsigned char)targetWord[currentGuess.size()]);
            currentGuess.push_back(c);
            gAudio.playPop();
            if (currentGuess == targetWord && !gGameData.levels.empty()) {
                gGameData.completeLevel(currentLevelIdx, 3);
                victoryModal.show(gGameData.levels[currentLevelIdx].word);
            }
        }
    }

    int update(float dt) {
        if (victoryModal.active) {
            victoryModal.update(dt);
            if (victoryModal.nextClicked) {
                setLevel((currentLevelIdx + 1) % std::max(1, gGameData.levelCount()));
                return -1;
            }
            if (victoryModal.mapClicked) {
                victoryModal.hide();
                return Nav::SCREEN_MAP;
            }
            return -1;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Pause / Back button
            if (CheckCollisionPointRec(m, Rectangle{ 18, 22, 60, 60 })) {
                gAudio.playClick();
                return Nav::SCREEN_MAP;
            }
            // Robot Helper / Speech Bubble Hint
            if (CheckCollisionPointRec(m, Rectangle{ 310, 95, 215, 110 })) {
                revealHint();
                return -1;
            }
            // Skip button
            if (CheckCollisionPointRec(m, Rectangle{ 372, 885, 145, 56 })) {
                setLevel((currentLevelIdx + 1) % std::max(1, gGameData.levelCount()));
                gAudio.playClick();
                return -1;
            }
            // 2x7 Keyboard Tiles
            for (int i = 0; i < 14 && i < (int)keys.size(); ++i) {
                int r = i / 7;
                int c = i % 7;
                Rectangle kRec{ 24.0f + c * 71.0f, 730.0f + r * 74.0f, 64.0f, 64.0f };
                if (!keyUsed[i] && CheckCollisionPointRec(m, kRec)) {
                    keyUsed[i] = true;
                    currentGuess.push_back(keys[i]);
                    gAudio.playPop();
                    if (currentGuess == targetWord && !gGameData.levels.empty()) {
                        gGameData.completeLevel(currentLevelIdx, 3);
                        victoryModal.show(gGameData.levels[currentLevelIdx].word);
                    } else if (currentGuess.size() >= targetWord.size()) {
                        currentGuess.clear();
                        std::fill(keyUsed.begin(), keyUsed.end(), false);
                    }
                    break;
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Academic Navy Background
        DrawRectangleGradientV(0, 0, w, h, Color{ 17, 28, 53, 255 }, Color{ 11, 19, 36, 255 });

        // 2. Top Pause Button, Pencil Progress Bar & Coin Pill
        UI::GradientRound({ 18, 22, 58, 58 }, 18.0f, Color{ 59, 130, 246, 255 }, Color{ 29, 78, 216, 255 });
        Theme::text3D("||", { 47, 51 }, 24, WHITE, BLACK, 2.0f, BLACK, true, true);

        // Pencil Progress Bar
        UI::GradientRound({ 110, 28, 280, 38 }, 14.0f, Color{ 245, 158, 11, 255 }, Color{ 180, 83, 9, 255 });
        UI::GradientRound({ 145, 34, 210, 26 }, 8.0f, Color{ 34, 197, 94, 255 }, Color{ 21, 128, 61, 255 });
        Theme::text3D("LEVEL " + std::to_string(currentLevelIdx + 1) + "/" + std::to_string(std::max(1, gGameData.levelCount())),
                      { 250, 80 }, 16, Color{ 253, 224, 71, 255 }, BLACK, 2.0f, BLACK, true, true);

        UI::GradientRound({ 412, 26, 112, 42 }, 18.0f, Color{ 15, 23, 42, 255 }, Color{ 30, 41, 59, 255 });
        Theme::text3D(std::to_string(Theme::coins), { 476, 47 }, 20, Color{ 253, 224, 71, 255 }, BLACK, 2.0f, BLACK, true, true);

        // 3. Ornate Academic Picture Frame + Clue Image
        Rectangle frameOuter{ 82, 182, 376, 396 };
        UI::DropShadow(frameOuter, 26.0f, 8.0f, 0.45f, 4.0f);
        UI::GradientRound(frameOuter, 26.0f, Color{ 251, 191, 36, 255 }, Color{ 180, 83, 9, 255 });
        Rectangle frameInner{ 102, 204, 336, 352 };
        UI::GradientRound(frameInner, 20.0f, Color{ 14, 165, 233, 255 }, Color{ 3, 105, 161, 255 });

        std::string imgKey = "assets/images/eclipse.png";
        if (!gGameData.levels.empty() && !gGameData.levels[currentLevelIdx].word.image_keyword.empty()) {
            imgKey = "assets/images/" + gGameData.levels[currentLevelIdx].word.image_keyword + ".png";
        }
        UIAssets::drawFit(imgKey, Rectangle{ 114, 218, 312, 324 });

        // 4. Animated Graduation Robot (60-frame sequence from robot_animation_studio_fixed)
        UI::GradientRound({ 305, 112, 118, 48 }, 16.0f, WHITE, Color{ 226, 232, 240, 255 });
        Theme::text3D("NEED A HINT?", { 364, 136 }, 14, Color{ 15, 23, 42, 255 }, WHITE, 1.0f, BLANK, true, true);

        Rectangle robotRec{ 412, 88, 118, 128 };
        if (!UIAssets::drawFrameSequence("frames_master/frame_", 60, t, 24.0f, robotRec)) {
            UIAssets::drawFit("clean_layers/body_armless_whole_visor.png", robotRec);
        }

        // 5. Dashed Answer Slots
        int n = (int)targetWord.size();
        float slotSz = std::min(58.0f, 460.0f / std::max(1, n));
        float totalW = n * (slotSz + 6.0f) - 6.0f;
        float startX = 270.0f - totalW * 0.5f;
        for (int i = 0; i < n; ++i) {
            Rectangle sRec{ startX + i * (slotSz + 6.0f), 618.0f, slotSz, slotSz };
            UI::GradientRound(sRec, 14.0f, Color{ 11, 19, 36, 255 }, Color{ 23, 37, 84, 255 });
            DrawRectangleRoundedLinesEx(sRec, 0.25f, 6, 2.5f, WHITE);
            if (i < (int)currentGuess.size()) {
                std::string k = std::string("word_graduate_victory_ui_kit/letter_tiles_a_z/tile_") + currentGuess[i] + ".png";
                UIAssets::drawFit(k, sRec);
            }
        }

        // 6. 2x7 3D Cream Keyboard Tiles + Shuffle & Skip Buttons
        for (int i = 0; i < 14 && i < (int)keys.size(); ++i) {
            if (keyUsed[i]) continue;
            int r = i / 7;
            int c = i % 7;
            Rectangle kRec{ 24.0f + c * 71.0f, 730.0f + r * 74.0f, 64.0f, 64.0f };
            std::string tileKey = std::string("word_graduate_victory_ui_kit/letter_tiles_a_z/tile_") + keys[i] + ".png";
            if (!UIAssets::drawFit(tileKey, kRec)) {
                UI::GradientRound(kRec, 14.0f, Color{ 255, 253, 245, 255 }, Color{ 254, 240, 138, 255 });
                Theme::text3D(std::string(1, keys[i]), { kRec.x + 32.0f, kRec.y + 32.0f }, 26, Color{ 15, 23, 42, 255 }, WHITE, 1.5f, BLANK, true, true);
            }
        }

        UI::GradientRound({ 372, 885, 145, 56 }, 20.0f, Color{ 251, 146, 60, 255 }, Color{ 239, 68, 68, 255 });
        Theme::text3D("SKIP >>|", { 444, 913 }, 20, WHITE, BLACK, 2.5f, BLACK, true, true);

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
```

---

### 3.10 الملف العاشر: `custom_engine/include/screen_wheel.hpp` *(تحديث كامل — عجلة الحروف مع رائد الفضاء المتحرك)*

استبدل محتوى الملف [`custom_engine/include/screen_wheel.hpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/include/screen_wheel.hpp) بالكود الكامل التالي:

```cpp
#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "victory_modal.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>

class ScreenWheel {
public:
    struct WheelNode {
        Vector2 pos{};
        char letter = 'A';
        bool selected = false;
    };

    VictoryModal victoryModal;
    std::string targetWord = "GALAXY";
    std::vector<WheelNode> nodes;
    std::vector<int> currentPath;
    int currentLevelIdx = 0;

    void init() {
        victoryModal.init();
        setLevel(0);
    }

    void setLevel(int index) {
        if (gGameData.levels.empty()) return;
        currentLevelIdx = std::clamp(index, 0, (int)gGameData.levels.size() - 1);
        targetWord = gGameData.levels[currentLevelIdx].word.word;
        victoryModal.hide();
        buildWheel();
    }

    void buildWheel() {
        nodes.clear();
        currentPath.clear();
        int n = std::max(3, (int)targetWord.size());
        Vector2 center{ 270.0f, 742.0f };
        float radius = 112.0f;
        for (int i = 0; i < n; ++i) {
            float ang = -PI * 0.5f + (2.0f * PI * i) / (float)n;
            WheelNode nd;
            nd.pos = { center.x + std::cos(ang) * radius, center.y + std::sin(ang) * radius };
            nd.letter = (char)std::toupper((unsigned char)targetWord[i]);
            nd.selected = false;
            nodes.push_back(nd);
        }
        // Pre-connect 3 nodes for visual laser demonstration
        if (nodes.size() >= 3) {
            currentPath = { 0, 1, 2 };
            nodes[0].selected = nodes[1].selected = nodes[2].selected = true;
        }
    }

    int update(float dt) {
        if (victoryModal.active) {
            victoryModal.update(dt);
            if (victoryModal.nextClicked) {
                setLevel((currentLevelIdx + 1) % std::max(1, gGameData.levelCount()));
                return -1;
            }
            if (victoryModal.mapClicked) {
                victoryModal.hide();
                return Nav::SCREEN_MAP;
            }
            return -1;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            if (CheckCollisionPointRec(m, Rectangle{ 458, 18, 60, 60 })) {
                gAudio.playClick();
                return Nav::SCREEN_MAP;
            }
            // Hint lightbulb (bottom right) completes word and opens VictoryModal
            if (CheckCollisionPointCircle(m, { 468, 900 }, 38.0f)) {
                if (!gGameData.levels.empty()) {
                    gGameData.completeLevel(currentLevelIdx, 3);
                    victoryModal.show(gGameData.levels[currentLevelIdx].word);
                }
                return -1;
            }
            // Clicking any wheel letter
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (CheckCollisionPointCircle(m, nodes[i].pos, 34.0f)) {
                    nodes[i].selected = true;
                    currentPath.push_back((int)i);
                    gAudio.playPop();
                    if (currentPath.size() >= nodes.size() && !gGameData.levels.empty()) {
                        gGameData.completeLevel(currentLevelIdx, 3);
                        victoryModal.show(gGameData.levels[currentLevelIdx].word);
                    }
                    break;
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Cosmic Observatory Backdrop + Telescope on Tripod
        DrawRectangleGradientV(0, 0, w, h, Color{ 10, 17, 40, 255 }, Color{ 28, 20, 72, 255 });
        UIAssets::drawCentered("astronomy_galaxy_word_wheel_ui_kit/05_word_wheel_and_screen_containers/bg_observatory_telescope_on_tripod.png",
                               { 88, 445 }, 165, 210);

        // 2. Top Header Banner, Coin Pill & Settings Gear
        UIAssets::drawFit("astronomy_galaxy_word_wheel_ui_kit/01_hud_and_action_buttons/hud_340_coin_counter_pill.png",
                          Rectangle{ 16, 20, 128, 48 });
        UIAssets::drawFit("astronomy_galaxy_word_wheel_ui_kit/05_word_wheel_and_screen_containers/header_level_42_astronomy_3d_banner.png",
                          Rectangle{ 148, 12, 295, 88 });
        UIAssets::drawFit("astronomy_galaxy_word_wheel_ui_kit/01_hud_and_action_buttons/btn_settings_blue_gear.png",
                          Rectangle{ 458, 18, 60, 60 });

        // 3. 3D White Crossword Grid Tiles (crossword_white_tile_A..Z)
        int n = (int)targetWord.size();
        float tileSz = std::min(58.0f, 380.0f / std::max(1, n));
        float totalW = n * (tileSz + 5.0f) - 5.0f;
        float startX = 250.0f - totalW * 0.5f;
        for (int i = 0; i < n; ++i) {
            Rectangle r{ startX + i * (tileSz + 5.0f), 255.0f, tileSz, tileSz };
            char c = (char)std::toupper((unsigned char)targetWord[i]);
            std::string key = std::string("04_white_crossword_tiles_a_to_z/crossword_white_tile_") + c + ".png";
            if (!UIAssets::drawFit(key, r)) {
                UIAssets::drawFit("04_white_crossword_tiles_a_to_z/white_crossword_tile_blank.png", r);
            }
        }

        // 4. Animated Celebrating Astronaut (60-frame master sequence)
        Rectangle astroRec{ 345, 345, 185, 215 };
        if (!UIAssets::drawFrameSequence("06_frames_master_astronaut/frame_", 60, t, 24.0f, astroRec)) {
            UIAssets::drawFit("02_celebrating_astronaut_animations_and_layers/astronaut_armless_pedestal_base.png", astroRec);
        }

        // 5. Glowing Cyan & Gold Crystal Word Wheel + Laser Connection Line + 3D Wheel Letters
        UIAssets::drawCentered("astronomy_galaxy_word_wheel_ui_kit/05_word_wheel_and_screen_containers/word_wheel_glowing_cyan_gold_blank.png",
                               { 270, 742 }, 365, 365);

        for (size_t i = 0; i + 1 < currentPath.size(); ++i) {
            Vector2 p1 = nodes[currentPath[i]].pos;
            Vector2 p2 = nodes[currentPath[i + 1]].pos;
            DrawLineEx(p1, p2, 12.0f, Color{ 6, 182, 212, 140 });
            DrawLineEx(p1, p2, 5.0f, Color{ 165, 243, 252, 255 });
        }

        const char* wheelColors[7] = {
            "_red.png", "_green.png", "_yellow.png", "_blue.png",
            "_purple.png", "_orange.png", "_teal.png"
        };
        for (size_t i = 0; i < nodes.size(); ++i) {
            char c = nodes[i].letter;
            std::string lKey = std::string("03_word_wheel_3d_letters_a_to_z/wheel_letter_") + c + wheelColors[(c - 'A') % 7];
            if (!UIAssets::drawCentered(lKey, nodes[i].pos, 68, 68)) {
                Theme::text3D(std::string(1, c), nodes[i].pos, 38, Color{ 253, 224, 71, 255 }, BLACK, 3.0f, BLACK, true, true);
            }
        }

        // 6. Bottom Corner Shuffle & Hint Buttons
        UIAssets::drawCentered("astronomy_galaxy_word_wheel_ui_kit/01_hud_and_action_buttons/btn_shuffle_blue_arrows.png",
                               { 72, 900 }, 78, 78);
        UIAssets::drawCentered("astronomy_galaxy_word_wheel_ui_kit/01_hud_and_action_buttons/btn_hint_gold_lightbulb.png",
                               { 468, 900 }, 78, 78);

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
```

---

### 3.11 الملف الحادي عشر: `custom_engine/src/main.cpp` *(تحديث كامل — ربط جميع الشاشات ووضع `--verify-ui`)*

استبدل محتوى الملف [`custom_engine/src/main.cpp`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/src/main.cpp) بالكود الكامل التالي:

```cpp
#include "raylib.h"
#include "rlgl.h"

#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "screen_crossword.hpp"
#include "screen_home.hpp"
#include "screen_image_guess.hpp"
#include "screen_maps.hpp"
#include "screen_shop.hpp"
#include "screen_streak.hpp"
#include "screen_wheel.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

constexpr int kWidth = 540;
constexpr int kHeight = 960;

void DrawFrame(ScreenHome& home, ScreenMaps& map, ScreenCrossword& crossword,
               ScreenImageGuess& imageGuess, ScreenWheel& wheel,
               ScreenShop& shop, ScreenStreak& streak,
               int screen, RenderTexture2D canvas) {
    BeginTextureMode(canvas);
    ClearBackground(Color{ 10, 17, 34, 255 });
    switch (screen) {
        case Nav::SCREEN_HOME: home.draw(kWidth, kHeight); break;
        case Nav::SCREEN_MAP: map.draw(kWidth, kHeight); break;
        case Nav::SCREEN_CROSSWORD: crossword.draw(kWidth, kHeight); break;
        case Nav::SCREEN_IMAGE_GUESS: imageGuess.draw(kWidth, kHeight); break;
        case Nav::SCREEN_WHEEL: wheel.draw(kWidth, kHeight); break;
        case Nav::SCREEN_SHOP: shop.draw(kWidth, kHeight); break;
        case Nav::SCREEN_STREAK: streak.draw(kWidth, kHeight); break;
        default: home.draw(kWidth, kHeight); break;
    }
    EndTextureMode();
}

void Capture(ScreenHome& home, ScreenMaps& map, ScreenCrossword& crossword,
             ScreenImageGuess& imageGuess, ScreenWheel& wheel,
             ScreenShop& shop, ScreenStreak& streak,
             int screen, const std::string& path, RenderTexture2D canvas) {
    Theme::timeNow += 0.35f;
    Theme::updateParticles(0.016f);
    for (int i = 0; i < 2; ++i) {
        DrawFrame(home, map, crossword, imageGuess, wheel, shop, streak, screen, canvas);
    }
    Image img = LoadImageFromTexture(canvas.texture);
    ImageFlipVertical(&img);
    ExportImage(img, path.c_str());
    UnloadImage(img);
    std::cout << "  [CAPTURED] " << path << std::endl;
}

int LevelOf(const std::string& word) {
    int idx = gGameData.indexOfWord(word);
    return (idx >= 0) ? idx : 0;
}

} // namespace

int main(int argc, char* argv[]) {
    bool verifyMode = false;
    bool selfTest = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verify-ui" || arg == "--verify") verifyMode = true;
        if (arg == "--self-test") selfTest = true;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "BAC Vocab Quest - VocabCross Engine");
    InitAudioDevice();
    SetTargetFPS(60);

    gAudio.init();
    Theme::initFonts();
    UIAssets::init();

    if (!gGameData.loadFromFile("assets/data/bac_vocabulary.json")) {
        gGameData.loadFromFile("../data/bac_vocabulary.json");
    }

    ScreenHome homeScreen;
    ScreenMaps mapScreen;
    ScreenCrossword crosswordScreen;
    ScreenImageGuess imageGuessScreen;
    ScreenWheel wheelScreen;
    ScreenShop shopScreen;
    ScreenStreak streakScreen;

    homeScreen.init();
    mapScreen.init();
    crosswordScreen.init();
    imageGuessScreen.init();
    wheelScreen.init();
    shopScreen.init();
    streakScreen.init();

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    if (verifyMode) {
        std::cout << "[VERIFY-UI] Capturing all 14 UI screens and modals..." << std::endl;
        std::filesystem::create_directories("verification_screenshots");

        // 0. Home Screen (BAC VOCAB QUEST + Animated Trio)
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_HOME, "verification_screenshots/00_home_bac_vocab_quest.png", canvas);

        // 1..3. Original 3 Unit Maps
        mapScreen.setUnit(0);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/01_map_ethics.png", canvas);

        mapScreen.setUnit(1);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/02_map_safety.png", canvas);

        mapScreen.setUnit(2);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/03_map_space.png", canvas);

        // 4. Graduation Academy Map (Scrolls 1-14 -> Grand Blue Castle)
        mapScreen.setUnit(3);
        mapScreen.boyProgress = 5.0f;
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/04_map_graduation_academy.png", canvas);

        // 5..6. Crossword (Level 1 & Level 2)
        crosswordScreen.setLevel(LevelOf("BRIBERY"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/04_crossword_lvl1.png", canvas);

        crosswordScreen.setLevel(LevelOf("COUNTERFEIT"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/05_crossword_lvl2.png", canvas);

        // 7..8. Image Guess (Level 1 & Level 2)
        imageGuessScreen.setLevel(LevelOf("ECLIPSE"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_IMAGE_GUESS, "verification_screenshots/06_image_guess_lvl1.png", canvas);

        imageGuessScreen.setLevel(LevelOf("ORGANIC"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_IMAGE_GUESS, "verification_screenshots/07_image_guess_lvl2.png", canvas);

        // 9. Wheel Connect
        wheelScreen.setLevel(LevelOf("GALAXY"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_WHEEL, "verification_screenshots/08_wheel_lvl1.png", canvas);

        // 10. Victory Modal (word_graduate_victory_ui_kit)
        if (!gGameData.levels.empty()) {
            crosswordScreen.victoryModal.show(gGameData.levels[LevelOf("BRIBERY")].word);
            crosswordScreen.victoryModal.anim = 1.0f;
        }
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/09_arabic_victory_modal.png", canvas);

        // 11. Word Card Modal ("INTEGRITY" - image1_integrity_word_card_ui_kit)
        WordData integrityWord;
        integrityWord.word = "INTEGRITY";
        integrityWord.translation_ar = "النزاهة / الاستقامة والأمانة";
        integrityWord.definition_en = "The quality of being honest and having strong moral principles.";
        integrityWord.synonym_en = "Honesty, Probity, Rectitude";
        integrityWord.frequency = "High Frequency - Ethics in Business";
        crosswordScreen.victoryModal.wordCardModal.show(integrityWord);
        crosswordScreen.victoryModal.wordCardModal.anim = 1.0f;
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/10_word_card_integrity.png", canvas);
        crosswordScreen.victoryModal.hide();

        // 12. Booster Shop (magical_alchemist_booster_shop_ui_kit)
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_SHOP, "verification_screenshots/11_booster_shop.png", canvas);

        // 13. Daily Streak 1-7 (daily_streak_roadmap_ui_kit)
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_STREAK, "verification_screenshots/12_daily_streak_1_to_7.png", canvas);

        UnloadRenderTexture(canvas);
        UIAssets::unloadAll();
        Theme::unloadFonts();
        gAudio.close();
        CloseAudioDevice();
        CloseWindow();
        return 0;
    }

    int screen = Nav::SCREEN_HOME;
    if (selfTest) screen = Nav::SCREEN_MAP;

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.05f);
        Theme::timeNow += dt;
        Theme::updateParticles(dt);

        Rectangle dst = Input::CanvasDestination(GetScreenWidth(), GetScreenHeight());
        Input::SetCanvasTransform(dst.width / (float)kWidth, dst.height / (float)kHeight, dst.x, dst.y);

        if (screen == Nav::SCREEN_HOME) {
            int next = homeScreen.update(dt);
            if (homeScreen.pendingAction == 2) screen = Nav::SCREEN_SHOP;
            else if (homeScreen.pendingAction == 3) screen = Nav::SCREEN_STREAK;
            else if (homeScreen.pendingAction == 4) {
                mapScreen.setUnit(3);
                screen = Nav::SCREEN_MAP;
            } else if (next >= 0) {
                screen = next;
            }
        } else if (screen == Nav::SCREEN_MAP) {
            int lvl = mapScreen.update(dt);
            if (mapScreen.pendingNavScreen >= 0) {
                screen = mapScreen.pendingNavScreen;
            } else if (lvl >= 0 && lvl < gGameData.levelCount()) {
                std::string m = gGameData.levels[lvl].mode;
                if (m == "image_guess") {
                    imageGuessScreen.setLevel(lvl);
                    screen = Nav::SCREEN_IMAGE_GUESS;
                } else if (m == "wheel") {
                    wheelScreen.setLevel(lvl);
                    screen = Nav::SCREEN_WHEEL;
                } else {
                    crosswordScreen.setLevel(lvl);
                    screen = Nav::SCREEN_CROSSWORD;
                }
            }
        } else if (screen == Nav::SCREEN_CROSSWORD) {
            int next = crosswordScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_IMAGE_GUESS) {
            int next = imageGuessScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_WHEEL) {
            int next = wheelScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_SHOP) {
            int next = shopScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_STREAK) {
            int next = streakScreen.update(dt);
            if (next >= 0) screen = next;
        }

        DrawFrame(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen,
                  shopScreen, streakScreen, screen, canvas);

        BeginDrawing();
        ClearBackground(Color{ 6, 10, 22, 255 });
        DrawTexturePro(canvas.texture,
                       Rectangle{ 0, 0, (float)kWidth, -(float)kHeight },
                       dst, Vector2{ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(canvas);
    UIAssets::unloadAll();
    Theme::unloadFonts();
    gAudio.close();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
```

---

### 3.12 الملف الثاني عشر: `custom_engine/verify_and_compare.py` *(تحديث سكريبت الفحص البصري ليشمل الشاشات الـ 14)*

استبدل محتوى الملف [`custom_engine/verify_and_compare.py`](https://github.com/Sibi9/VocabCross/blob/main/custom_engine/verify_and_compare.py) بالكود التالي:

```python
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
```

---

## 4. أوامر البناء والتشغيل والتحقق الإلزامي للوكيل (Build & Verification Commands)

بعد حفظ الملفات أعلاه، نفّذ الأوامر التالية بالترتيب داخل مجلد `custom_engine/`:

```powershell
# 1. فك ضغط أي حزم مضغوطة داخل ui assest ومزامنة الملفات
python ../apply_ui_assets_update.py

# 2. ترجمة المحرك وبناء VocabCross.exe
.\build.bat

# 3. تشغيل الالتقاط الآلي لجميع الشاشات الـ 14 (بما فيها شاشة البداية، المتجر، المثابرة 1-7، بطاقة الكلمة، وخريطة الأكاديمية 1-14)
.\VocabCross.exe --verify-ui

# 4. تشغيل سكريبت التحقق البصري وتوليد لوحة المعاينة الشاملة
python verify_and_compare.py
```
