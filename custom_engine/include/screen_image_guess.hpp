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
    UI::StatefulButton backBtn;
    UI::StatefulButton shuffleBtn;
    UI::StatefulButton hintBtn;
    UI::StatefulButton skipBtn;

    std::string targetWord = "ECLIPSE";
    std::vector<char> slotChars;
    std::vector<int> slotKeySource; // index into keys[]
    std::vector<bool> slotLocked;
    std::vector<char> keys;
    std::vector<bool> keyUsed;
    float keyScales[14]{};
    float keyOffsets[14]{};
    int currentLevelIdx = 0;

    void init() {
        victoryModal.init();

        backBtn.bounds = Rectangle{ 12.0f, 12.0f, 88.0f, 40.0f };
        backBtn.label = "< MAP";
        backBtn.cornerRadius = 12.0f;
        backBtn.fontSize = 14.0f;
        backBtn.topColor = Color{ 16, 185, 129, 245 };
        backBtn.bottomColor = Color{ 5, 150, 105, 245 };
        backBtn.lipColor = Color{ 6, 95, 70, 245 };
        backBtn.borderColor = Color{ 167, 243, 208, 255 };

        shuffleBtn.bounds = Rectangle{ 20.0f, 880.0f, 115.0f, 48.0f };
        shuffleBtn.label = "SHUFFLE";
        shuffleBtn.cornerRadius = 14.0f;
        shuffleBtn.fontSize = 13.0f;
        shuffleBtn.topColor = Color{ 139, 92, 246, 245 };
        shuffleBtn.bottomColor = Color{ 109, 40, 217, 245 };
        shuffleBtn.lipColor = Color{ 91, 33, 182, 245 };
        shuffleBtn.borderColor = Color{ 221, 214, 254, 255 };

        hintBtn.bounds = Rectangle{ 210.0f, 880.0f, 120.0f, 48.0f };
        hintBtn.label = "HINT";
        hintBtn.cornerRadius = 14.0f;
        hintBtn.fontSize = 13.0f;
        hintBtn.topColor = Color{ 245, 158, 11, 245 };
        hintBtn.bottomColor = Color{ 180, 83, 9, 245 };
        hintBtn.lipColor = Color{ 120, 53, 15, 245 };
        hintBtn.borderColor = Color{ 254, 240, 138, 255 };

        skipBtn.bounds = Rectangle{ 405.0f, 880.0f, 115.0f, 48.0f };
        skipBtn.label = "NEXT";
        skipBtn.cornerRadius = 14.0f;
        skipBtn.fontSize = 13.0f;
        skipBtn.topColor = Color{ 37, 99, 235, 245 };
        skipBtn.bottomColor = Color{ 30, 58, 138, 245 };
        skipBtn.lipColor = Color{ 23, 37, 84, 245 };
        skipBtn.borderColor = Color{ 191, 219, 254, 255 };

        for (int i = 0; i < 14; ++i) {
            keyScales[i] = 1.0f;
            keyOffsets[i] = 0.0f;
        }

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

        Vector2 m = Input::Mouse();
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        backBtn.update(m, mouseDown, dt);
        shuffleBtn.update(m, mouseDown, dt);
        hintBtn.update(m, mouseDown, dt);
        skipBtn.update(m, mouseDown, dt);

        if (backBtn.isClicked(m, mouseReleased)) {
            gAudio.playClick();
            return Nav::SCREEN_MAP;
        }

        if (shuffleBtn.isClicked(m, mouseReleased)) {
            shuffleUnusedKeys();
            return -1;
        }

        if (hintBtn.isClicked(m, mouseReleased)) {
            revealHint();
            return -1;
        }

        if (skipBtn.isClicked(m, mouseReleased)) {
            setLevel((currentLevelIdx + 1) % std::max(1, gGameData.levelCount()));
            gAudio.playClick();
            return -1;
        }

        // Keyboard tile spring physics
        for (int i = 0; i < 14 && i < (int)keys.size(); ++i) {
            if (keyUsed[i]) {
                keyScales[i] += (0.82f - keyScales[i]) * std::min(1.0f, dt * 16.0f);
                keyOffsets[i] += (0.0f - keyOffsets[i]) * std::min(1.0f, dt * 16.0f);
                continue;
            }
            int r = i / 7;
            int c = i % 7;
            Rectangle kRec{ 24.0f + c * 71.0f, 718.0f + r * 74.0f, 64.0f, 64.0f };
            bool hov = CheckCollisionPointRec(m, kRec);
            float targetS = 1.0f;
            float targetO = 0.0f;
            if (hov) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                targetS = mouseDown ? 0.95f : 1.08f;
                targetO = mouseDown ? 3.0f : 0.0f;
            }
            keyScales[i] += (targetS - keyScales[i]) * std::min(1.0f, dt * 18.0f);
            keyOffsets[i] += (targetO - keyOffsets[i]) * std::min(1.0f, dt * 20.0f);
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
            // Robot Helper / Speech Bubble Hint
            if (CheckCollisionPointRec(m, Rectangle{ 390, 75, 140, 145 })) {
                revealHint();
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

        // Top HUD Bar
        backBtn.draw();

        Rectangle lvlCapsule{ 170, 12, 200, 40 };
        UI::GlassPanel(lvlCapsule, 14.0f, Color{ 30, 41, 59, 230 }, Color{ 148, 163, 184, 255 });
        std::string lvlStr = "LEVEL " + std::to_string(currentLevelIdx + 1) + " / " + std::to_string(std::max(1, gGameData.levelCount()));
        Theme::text3D(lvlStr, { 270, 32 }, 15, Color{ 253, 224, 71, 255 }, BLACK, 2.0f, BLACK, true, true);

        Rectangle coinPill{ 418, 12, 110, 40 };
        UI::GlassPanel(coinPill, 14.0f, Color{ 30, 41, 59, 230 }, Color{ 250, 204, 21, 255 });
        UI::CoinIcon({ 436, 32 }, 12.0f);
        Theme::text3D(std::to_string(Theme::coins), { 482, 32 }, 16, WHITE, BLACK, 1.8f, BLACK, true, true);

        // Interactive Image Card (with hover zoom to inspect details)
        Rectangle imgBaseRec{ 112, 226, 316, 308 };
        bool imgHov = CheckCollisionPointRec(m, imgBaseRec);
        if (imgHov) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        float imgZoom = imgHov ? 1.03f : 1.0f;
        Rectangle imgDrawRec{
            270.0f - 158.0f * imgZoom,
            380.0f - 154.0f * imgZoom,
            316.0f * imgZoom,
            308.0f * imgZoom
        };
        UI::DropShadow(imgDrawRec, 18.0f, 6.0f, 0.40f, 3.0f);

        std::string imgKey = "images/eclipse.png";
        if (!gGameData.levels.empty() && !gGameData.levels[currentLevelIdx].word.image_keyword.empty()) {
            std::string cand = "images/" + gGameData.levels[currentLevelIdx].word.image_keyword + ".png";
            if (UIAssets::has(cand)) imgKey = cand;
        }
        UIAssets::drawStretch(imgKey, imgDrawRec);
        UI::Rim(imgDrawRec, 18.0f, imgHov ? Color{ 254, 240, 138, 255 } : Color{ 148, 163, 184, 220 },
                WHITE, Color{ 30, 41, 59, 220 });

        // Animated Robot Assistant with bobbing motion
        float robotBob = std::sin(t * 3.0f) * 5.0f;
        Rectangle robotRec{ 396, 72 + robotBob, 136, 180 };
        UIAssets::drawFrameSequence("image_guess/robot_rgba/frame_", 16, t, 12.0f, robotRec);

        // Answer Slots
        int n = std::max(3, (int)targetWord.size());
        float slotSz = std::min(56.0f, 460.0f / (float)n);
        float totalW = n * (slotSz + 6.0f) - 6.0f;
        float startX = 270.0f - totalW * 0.5f;
        for (int i = 0; i < (int)slotChars.size(); ++i) {
            Rectangle sRec{ startX + i * (slotSz + 6.0f), 606.0f, slotSz, slotSz };
            bool slotHov = (!slotLocked[i] && slotChars[i] != '_' && CheckCollisionPointRec(m, sRec));
            if (slotHov) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                sRec.y -= 2.0f;
            }
            UI::DropShadow(sRec, 14.0f, 3.0f, 0.30f, 2.0f);
            UI::GradientRound(sRec, 14.0f, Color{ 11, 19, 36, 255 }, Color{ 23, 37, 84, 255 });
            UI::Rim(sRec, 14.0f, Color{ 148, 163, 184, 220 }, WHITE, Color{ 51, 65, 85, 220 });
            if (slotChars[i] != '_') {
                std::string k = std::string("image_guess/cream_tiles/tile_") + slotChars[i] + ".png";
                UIAssets::drawFit(k, sRec);
            }
        }

        // 2x7 Keyboard Tiles with 4-state visual machine
        for (int i = 0; i < 14 && i < (int)keys.size(); ++i) {
            if (keyUsed[i]) continue;
            int r = i / 7;
            int c = i % 7;
            float baseW = 64.0f;
            float baseH = 64.0f;
            float cx = 24.0f + c * 71.0f + baseW * 0.5f;
            float cy = 718.0f + r * 74.0f + baseH * 0.5f + keyOffsets[i];
            float drawW = baseW * keyScales[i];
            float drawH = baseH * keyScales[i];
            Rectangle kRec{ cx - drawW * 0.5f, cy - drawH * 0.5f, drawW, drawH };
            UI::DropShadow(kRec, 12.0f, 3.0f * keyScales[i], 0.35f, 2.0f);
            std::string tileKey = std::string("image_guess/cream_tiles/tile_") + keys[i] + ".png";
            UIAssets::drawFit(tileKey, kRec);
        }

        // Bottom Action Buttons (Stateful Buttons)
        shuffleBtn.draw();
        hintBtn.draw();
        skipBtn.draw();

        Theme::drawParticles();

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
