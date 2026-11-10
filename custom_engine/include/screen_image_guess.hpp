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
            DrawRectangleRoundedLines(sRec, 0.25f, 6, 2.5f, WHITE);
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
