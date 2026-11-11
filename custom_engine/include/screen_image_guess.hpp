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
    std::vector<char> slotChars;
    std::vector<int> slotKeySource; // index into keys[]
    std::vector<bool> slotLocked;
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
        victoryModal.hide();

        int n = std::max(1, (int)targetWord.size());
        slotChars.assign(n, '_');
        slotKeySource.assign(n, -1);
        slotLocked.assign(n, false);

        keys.clear();
        keyUsed.clear();
        for (char c : targetWord) {
            if (std::isalpha((unsigned char)c)) keys.push_back((char)std::toupper((unsigned char)c));
        }
        const std::string extra = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        while (keys.size() < 14) {
            keys.push_back(extra[(keys.size() * 5 + currentLevelIdx * 3) % 26]);
        }
        for (size_t i = 0; i < keys.size(); ++i) {
            size_t j = (i * 5 + currentLevelIdx * 3 + 2) % keys.size();
            std::swap(keys[i], keys[j]);
        }
        keyUsed.assign(keys.size(), false);
    }

    void checkCompletion() {
        int n = (int)targetWord.size();
        for (int i = 0; i < n; ++i) {
            if (slotChars[i] == '_') return;
        }
        std::string built(slotChars.begin(), slotChars.end());
        if (built == targetWord && !gGameData.levels.empty()) {
            gGameData.completeLevel(currentLevelIdx, 3);
            victoryModal.show(gGameData.levels[currentLevelIdx].word);
        } else {
            // Clear non-locked slots so player can retry
            gAudio.playPop();
            for (int i = 0; i < n; ++i) {
                if (!slotLocked[i]) {
                    if (slotKeySource[i] >= 0 && slotKeySource[i] < (int)keyUsed.size()) {
                        keyUsed[slotKeySource[i]] = false;
                    }
                    slotChars[i] = '_';
                    slotKeySource[i] = -1;
                }
            }
        }
    }

    void revealHint() {
        int n = (int)targetWord.size();
        for (int i = 0; i < n; ++i) {
            char expected = (char)std::toupper((unsigned char)targetWord[i]);
            if (!slotLocked[i] || slotChars[i] != expected) {
                if (slotKeySource[i] >= 0 && slotKeySource[i] < (int)keyUsed.size()) {
                    keyUsed[slotKeySource[i]] = false;
                }
                slotChars[i] = expected;
                slotLocked[i] = true;
                slotKeySource[i] = -1;
                for (size_t k = 0; k < keys.size(); ++k) {
                    if (!keyUsed[k] && keys[k] == expected) {
                        keyUsed[k] = true;
                        slotKeySource[i] = (int)k;
                        break;
                    }
                }
                gAudio.playPop();
                checkCompletion();
                return;
            }
        }
    }

    void shuffleUnusedKeys() {
        for (size_t i = 0; i < keys.size(); ++i) {
            if (keyUsed[i]) continue;
            size_t j = (i * 3 + (int)(Theme::timeNow * 10.0f) + 1) % keys.size();
            if (!keyUsed[j]) {
                std::swap(keys[i], keys[j]);
            }
        }
        gAudio.playClick();
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

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) {
            for (int i = (int)slotChars.size() - 1; i >= 0; --i) {
                if (!slotLocked[i] && slotChars[i] != '_') {
                    if (slotKeySource[i] >= 0 && slotKeySource[i] < (int)keyUsed.size()) {
                        keyUsed[slotKeySource[i]] = false;
                    }
                    slotChars[i] = '_';
                    slotKeySource[i] = -1;
                    gAudio.playClick();
                    break;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Pause / Back button
            if (CheckCollisionPointRec(m, Rectangle{ 12, 16, 68, 68 })) {
                gAudio.playClick();
                return Nav::SCREEN_MAP;
            }
            // Robot Helper / Speech Bubble Hint
            if (CheckCollisionPointRec(m, Rectangle{ 290, 75, 240, 145 })) {
                revealHint();
                return -1;
            }
            // Shuffle button (bottom-left)
            if (CheckCollisionPointRec(m, Rectangle{ 16, 875, 118, 70 })) {
                shuffleUnusedKeys();
                return -1;
            }
            // Skip button (bottom-right)
            if (CheckCollisionPointRec(m, Rectangle{ 368, 878, 155, 65 })) {
                setLevel((currentLevelIdx + 1) % std::max(1, gGameData.levelCount()));
                gAudio.playClick();
                return -1;
            }
            // Clicking any filled answer slot removes that letter (Undo!)
            int n = std::max(3, (int)targetWord.size());
            float slotSz = std::min(56.0f, 460.0f / (float)n);
            float totalW = n * (slotSz + 6.0f) - 6.0f;
            float startX = 270.0f - totalW * 0.5f;
            for (int i = 0; i < (int)slotChars.size(); ++i) {
                Rectangle sRec{ startX + i * (slotSz + 6.0f), 606.0f, slotSz, slotSz };
                if (!slotLocked[i] && slotChars[i] != '_' && CheckCollisionPointRec(m, sRec)) {
                    if (slotKeySource[i] >= 0 && slotKeySource[i] < (int)keyUsed.size()) {
                        keyUsed[slotKeySource[i]] = false;
                    }
                    slotChars[i] = '_';
                    slotKeySource[i] = -1;
                    gAudio.playClick();
                    return -1;
                }
            }

            // 2x7 Keyboard Tiles
            for (int i = 0; i < 14 && i < (int)keys.size(); ++i) {
                int r = i / 7;
                int c = i % 7;
                Rectangle kRec{ 24.0f + c * 71.0f, 718.0f + r * 74.0f, 64.0f, 64.0f };
                if (!keyUsed[i] && CheckCollisionPointRec(m, kRec)) {
                    for (size_t s = 0; s < slotChars.size(); ++s) {
                        if (slotChars[s] == '_') {
                            keyUsed[i] = true;
                            slotChars[s] = keys[i];
                            slotKeySource[s] = i;
                            gAudio.playPop();
                            checkCompletion();
                            break;
                        }
                    }
                    break;
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;
        Vector2 m = Input::Mouse();

        if (!UIAssets::drawStretch("image_guess/bg_image_guess_clean.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 17, 28, 53, 255 }, Color{ 11, 19, 36, 255 });
        }

        Theme::text3D("LEVEL " + std::to_string(currentLevelIdx + 1) + " / " + std::to_string(std::max(1, gGameData.levelCount())),
                      { 270, 82 }, 15, Color{ 253, 224, 71, 255 }, BLACK, 2.0f, BLACK, true, true);
        Theme::text3D(std::to_string(Theme::coins), { 488, 42 }, 19, WHITE, BLACK, 2.0f, BLACK, true, true);

        std::string imgKey = "images/eclipse.png";
        if (!gGameData.levels.empty() && !gGameData.levels[currentLevelIdx].word.image_keyword.empty()) {
            std::string cand = "images/" + gGameData.levels[currentLevelIdx].word.image_keyword + ".png";
            if (UIAssets::has(cand)) imgKey = cand;
        }
        UIAssets::drawStretch(imgKey, Rectangle{ 112, 226, 316, 308 });

        Rectangle robotRec{ 396, 72, 136, 180 };
        UIAssets::drawFrameSequence("image_guess/robot_rgba/frame_", 16, t, 12.0f, robotRec);

        int n = std::max(3, (int)targetWord.size());
        float slotSz = std::min(56.0f, 460.0f / (float)n);
        float totalW = n * (slotSz + 6.0f) - 6.0f;
        float startX = 270.0f - totalW * 0.5f;
        for (int i = 0; i < (int)slotChars.size(); ++i) {
            Rectangle sRec{ startX + i * (slotSz + 6.0f), 606.0f, slotSz, slotSz };
            UI::GradientRound(sRec, 14.0f, Color{ 11, 19, 36, 255 }, Color{ 23, 37, 84, 255 });
            UI::Rim(sRec, 14.0f, Color{ 148, 163, 184, 220 }, WHITE, Color{ 51, 65, 85, 220 });
            if (slotChars[i] != '_') {
                std::string k = std::string("image_guess/cream_tiles/tile_") + slotChars[i] + ".png";
                UIAssets::drawFit(k, sRec);
            }
        }

        for (int i = 0; i < 14 && i < (int)keys.size(); ++i) {
            if (keyUsed[i]) continue;
            int r = i / 7;
            int c = i % 7;
            Rectangle kRec{ 24.0f + c * 71.0f, 718.0f + r * 74.0f, 64.0f, 64.0f };
            if (CheckCollisionPointRec(m, kRec)) {
                kRec.y -= 3.0f;
            }
            std::string tileKey = std::string("image_guess/cream_tiles/tile_") + keys[i] + ".png";
            UIAssets::drawFit(tileKey, kRec);
        }

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
