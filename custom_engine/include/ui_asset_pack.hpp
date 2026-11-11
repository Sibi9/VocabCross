#pragma once

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
