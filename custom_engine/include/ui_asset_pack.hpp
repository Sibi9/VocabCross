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
    if (!std::filesystem::exists(rootDir, ec) || !std::filesystem::is_directory(rootDir, ec)) return;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 rootDir, std::filesystem::directory_options::skip_permission_denied, ec)) {
            if (ec) { ec.clear(); continue; }
            std::error_code rec;
            if (!entry.is_regular_file(rec) || rec) continue;

            std::string pathStr = entry.path().string();
            if (pathStr.find(".git") != std::string::npos) continue;

            auto ext = toLower(entry.path().extension().string());
            if (ext != ".png" && ext != ".jpg" && ext != ".jpeg") continue;

            std::string fullPath = normalizeSlashes(pathStr);
            std::string baseName = toLower(entry.path().filename().string());
            std::error_code relEc;
            std::string relPath = toLower(normalizeSlashes(std::filesystem::relative(entry.path(), rootDir, relEc).string()));

            // Store by relative path, parent+filename, and basename
            gPathIndex[relPath] = fullPath;
            if (gPathIndex.find(baseName) == gPathIndex.end()) {
                gPathIndex[baseName] = fullPath;
            }
            auto parentName = toLower(entry.path().parent_path().filename().string());
            if (!parentName.empty()) {
                gPathIndex[parentName + "/" + baseName] = fullPath;
            }
        }
    } catch (...) {
        // Catch any filesystem exception safely
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

    // Fallback 1: search by suffix match
    for (const auto& kv : gPathIndex) {
        if (kv.first.size() >= norm.size() &&
            kv.first.compare(kv.first.size() - norm.size(), norm.size(), norm) == 0) {
            return kv.second;
        }
    }

    // Fallback 2: search by filename without directory
    auto slash = norm.find_last_of('/');
    std::string base = (slash != std::string::npos) ? norm.substr(slash + 1) : norm;
    auto bit = gPathIndex.find(base);
    if (bit != gPathIndex.end()) return bit->second;

    // Intelligent aliases for home screen & trio assets
    static const std::map<std::string, std::string> aliases = {
        { "emblem_bac_vocab_quest_crest.png", "prop_graduation_academy_banner.png" },
        { "btn_play_emerald_gold_with_text.png", "btn_next_level_cta.png" },
        { "scholar_owl_body_clean.png", "char_professor_owl_pose_a.png" },
        { "scholar_owl_waving_wing.png", "char_owl_pose2_flapping.png" },
        { "cheering_fox_full_rgba.png", "fox_base_body_clean.png" },
        { "astronaut_body_armless.png", "astronaut_armless_pedestal_base.png" },
        { "astronaut_left_arm.png", "astronaut_left_arm_clean.png" },
        { "astronaut_right_arm.png", "astronaut_right_arm_clean.png" },
        { "island_01_observatory_telescope.png", "landmark_02_observatory_dome_and_telescope.png" },
        { "island_02_classical_greek_temple.png", "temple_courthouse.png" },
        { "island_03_open_book_fairytale_castle.png", "bridge_giant_open_book.png" },
        { "flying_book_01_cream.png", "prop_flying_book_pose1.png" },
        { "flying_book_02_purple.png", "prop_flying_book_pose2.png" },
        { "flying_book_03_red.png", "prop_flying_book_pose1.png" },
        { "hud_avatar_lv1_badge.png", "badge_streak_12_days.png" },
        { "hud_heart_life_bar.png", "hud_progress_bar_pencil_complete.png" },
        { "btn_sound_speaker.png", "btn_audio_speaker.png" },
        { "btn_settings_gear.png", "btn_settings_blue_gear.png" }
    };

    auto ait = aliases.find(base);
    if (ait != aliases.end()) {
        auto fit = gPathIndex.find(ait->second);
        if (fit != gPathIndex.end()) return fit->second;
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
        } else {
            std::string fb1 = "letter_tiles_a_z/tile_" + std::string(1, clean[i]) + ".png";
            std::string fb2 = "alphabet_tiles_a_to_z/letter_" + std::string(1, clean[i]) + ".png";
            if (drawFit(fb1, r) || drawFit(fb2, r)) {
                anyDrawn = true;
            }
        }
    }
    return anyDrawn;
}

} // namespace UIAssets
