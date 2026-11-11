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
    std::string currentGuess;
    std::vector<WheelNode> nodes;
    std::vector<int> currentPath;
    int currentLevelIdx = 0;
    bool dragging = false;

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
        buildWheel();
    }

    void buildWheel() {
        nodes.clear();
        currentPath.clear();
        currentGuess.clear();
        dragging = false;

        int n = std::max(3, (int)targetWord.size());
        Vector2 center{ 270.0f, 735.0f };
        float radius = 108.0f;
        for (int i = 0; i < n; ++i) {
            float ang = -PI * 0.5f + (2.0f * PI * i) / (float)n;
            WheelNode nd;
            nd.pos = { center.x + std::cos(ang) * radius, center.y + std::sin(ang) * radius };
            nd.letter = (char)std::toupper((unsigned char)targetWord[i]);
            nd.selected = false;
            nodes.push_back(nd);
        }
    }

    void checkWord() {
        if (currentGuess.size() < targetWord.size()) return;
        if (currentGuess == targetWord && !gGameData.levels.empty()) {
            gGameData.completeLevel(currentLevelIdx, 3);
            victoryModal.show(gGameData.levels[currentLevelIdx].word);
        } else {
            gAudio.playPop();
            currentPath.clear();
            currentGuess.clear();
            for (auto& nd : nodes) nd.selected = false;
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

        Vector2 m = Input::Mouse();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(m, Rectangle{ 458, 12, 68, 68 })) {
                gAudio.playClick();
                return Nav::SCREEN_MAP;
            }
            // Shuffle button (bottom-left)
            if (CheckCollisionPointCircle(m, { 68, 896 }, 40.0f)) {
                currentPath.clear();
                currentGuess.clear();
                for (auto& nd : nodes) nd.selected = false;
                gAudio.playClick();
                return -1;
            }
            // Hint lightbulb (bottom-right) reveals next letter
            if (CheckCollisionPointCircle(m, { 472, 896 }, 40.0f)) {
                if (currentGuess.size() < targetWord.size()) {
                    char nextChar = (char)std::toupper((unsigned char)targetWord[currentGuess.size()]);
                    for (size_t i = 0; i < nodes.size(); ++i) {
                        if (!nodes[i].selected && nodes[i].letter == nextChar) {
                            nodes[i].selected = true;
                            currentPath.push_back((int)i);
                            currentGuess.push_back(nextChar);
                            gAudio.playPop();
                            checkWord();
                            break;
                        }
                    }
                }
                return -1;
            }

            for (size_t i = 0; i < nodes.size(); ++i) {
                if (CheckCollisionPointCircle(m, nodes[i].pos, 36.0f)) {
                    dragging = true;
                    if (!nodes[i].selected) {
                        nodes[i].selected = true;
                        currentPath.push_back((int)i);
                        currentGuess.push_back(nodes[i].letter);
                        gAudio.playPop();
                        checkWord();
                    }
                    break;
                }
            }
        }

        if (dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (!nodes[i].selected && CheckCollisionPointCircle(m, nodes[i].pos, 32.0f)) {
                    nodes[i].selected = true;
                    currentPath.push_back((int)i);
                    currentGuess.push_back(nodes[i].letter);
                    gAudio.playPop();
                    checkWord();
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            dragging = false;
        }

        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        if (!UIAssets::drawStretch("wheel/bg_wheel_clean.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 10, 17, 40, 255 }, Color{ 28, 20, 72, 255 });
        }

        int n = std::max(3, (int)targetWord.size());
        float tileSz = std::min(52.0f, 330.0f / (float)n);
        float gap = 4.0f;
        float totalW = n * (tileSz + gap) - gap;
        float startX = 215.0f - totalW * 0.5f;
        float rowY = 235.0f;

        for (int i = 0; i < n; ++i) {
            Rectangle r{ startX + i * (tileSz + gap), rowY, tileSz, tileSz };
            if (i < (int)currentGuess.size()) {
                char c = (char)std::toupper((unsigned char)currentGuess[i]);
                std::string key = std::string("wheel/white_tiles/crossword_white_tile_") + c + ".png";
                UIAssets::drawFit(key, r);
            } else {
                UIAssets::drawFit("wheel/white_tiles/white_crossword_tile_blank.png", r);
            }
        }

        Rectangle astroRec{ 346, 336, 178, 212 };
        UIAssets::drawFrameSequence("wheel/astro_rgba/frame_", 16, t, 12.0f, astroRec);

        // Live Word Preview Pill above the wheel
        if (!currentGuess.empty()) {
            Rectangle pill{ 160, 520, 220, 40 };
            UI::GradientRound(pill, 16.0f, Color{ 6, 182, 212, 230 }, Color{ 14, 116, 144, 230 });
            UI::Rim(pill, 16.0f, WHITE, WHITE, Color{ 165, 243, 252, 255 });
            Theme::text3D(currentGuess, { 270, 540 }, 22, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        for (size_t i = 0; i + 1 < currentPath.size(); ++i) {
            Vector2 p1 = nodes[currentPath[i]].pos;
            Vector2 p2 = nodes[currentPath[i + 1]].pos;
            DrawLineEx(p1, p2, 12.0f, Color{ 6, 182, 212, 145 });
            DrawLineEx(p1, p2, 5.0f, Color{ 165, 243, 252, 255 });
        }

        const char* wheelColors[7] = {
            "_red.png", "_green.png", "_yellow.png", "_blue.png",
            "_purple.png", "_orange.png", "_teal.png"
        };
        for (size_t i = 0; i < nodes.size(); ++i) {
            char c = nodes[i].letter;
            std::string lKey = std::string("wheel/wheel_letters/wheel_letter_") + c + wheelColors[(c - 'A') % 7];
            float sz = nodes[i].selected ? 74.0f : 66.0f;
            UIAssets::drawCentered(lKey, nodes[i].pos, sz, sz);
        }

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
