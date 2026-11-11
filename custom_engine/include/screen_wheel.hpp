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
        float currentScale = 1.0f;
        float targetScale = 1.0f;
        float hoverGlow = 0.0f;
        bool isHovered = false;
    };

    VictoryModal victoryModal;
    UI::StatefulButton backBtn;
    UI::StatefulButton shuffleBtn;
    UI::StatefulButton hintBtn;

    std::string targetWord = "GALAXY";
    std::string currentGuess;
    std::vector<WheelNode> nodes;
    std::vector<int> currentPath;
    int currentLevelIdx = 0;
    bool dragging = false;

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

        shuffleBtn.bounds = Rectangle{ 24.0f, 880.0f, 115.0f, 48.0f };
        shuffleBtn.label = "SHUFFLE";
        shuffleBtn.cornerRadius = 14.0f;
        shuffleBtn.fontSize = 14.0f;
        shuffleBtn.topColor = Color{ 139, 92, 246, 245 };
        shuffleBtn.bottomColor = Color{ 109, 40, 217, 245 };
        shuffleBtn.lipColor = Color{ 91, 33, 182, 245 };
        shuffleBtn.borderColor = Color{ 221, 214, 254, 255 };

        hintBtn.bounds = Rectangle{ 400.0f, 880.0f, 115.0f, 48.0f };
        hintBtn.label = "HINT";
        hintBtn.cornerRadius = 14.0f;
        hintBtn.fontSize = 14.0f;
        hintBtn.topColor = Color{ 245, 158, 11, 245 };
        hintBtn.bottomColor = Color{ 180, 83, 9, 245 };
        hintBtn.lipColor = Color{ 120, 53, 15, 245 };
        hintBtn.borderColor = Color{ 254, 240, 138, 255 };

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
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        backBtn.update(m, mouseDown, dt);
        shuffleBtn.update(m, mouseDown, dt);
        hintBtn.update(m, mouseDown, dt);

        if (backBtn.isClicked(m, mouseReleased)) {
            gAudio.playClick();
            return Nav::SCREEN_MAP;
        }

        if (shuffleBtn.isClicked(m, mouseReleased)) {
            currentPath.clear();
            currentGuess.clear();
            for (auto& nd : nodes) nd.selected = false;
            // Deterministic rotation of wheel node letters
            if (nodes.size() > 1) {
                char first = nodes[0].letter;
                for (size_t i = 0; i + 1 < nodes.size(); ++i) {
                    nodes[i].letter = nodes[i + 1].letter;
                }
                nodes.back().letter = first;
            }
            gAudio.playClick();
            return -1;
        }

        if (hintBtn.isClicked(m, mouseReleased)) {
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

        // Wheel letter node spring physics
        for (size_t i = 0; i < nodes.size(); ++i) {
            auto& nd = nodes[i];
            nd.isHovered = CheckCollisionPointCircle(m, nd.pos, 36.0f);
            if (nd.selected) {
                nd.targetScale = 1.15f;
                nd.hoverGlow = 1.0f;
            } else if (nd.isHovered) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                nd.targetScale = mouseDown ? 0.95f : 1.10f;
                nd.hoverGlow = 0.85f;
            } else {
                nd.targetScale = 1.0f;
                nd.hoverGlow = 0.0f;
            }
            nd.currentScale += (nd.targetScale - nd.currentScale) * std::min(1.0f, dt * 18.0f);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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

        if (dragging && mouseDown) {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (!nodes[i].selected && CheckCollisionPointCircle(m, nodes[i].pos, 34.0f)) {
                    nodes[i].selected = true;
                    currentPath.push_back((int)i);
                    currentGuess.push_back(nodes[i].letter);
                    gAudio.playPop();
                    checkWord();
                    break;
                }
            }
        }

        if (mouseReleased) {
            dragging = false;
        }

        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;
        Vector2 m = Input::Mouse();

        if (!UIAssets::drawStretch("wheel/bg_wheel_clean.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 10, 17, 40, 255 }, Color{ 28, 20, 72, 255 });
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

        int n = std::max(3, (int)targetWord.size());
        float tileSz = std::min(52.0f, 330.0f / (float)n);
        float gap = 4.0f;
        float totalW = n * (tileSz + gap) - gap;
        float startX = 215.0f - totalW * 0.5f;
        float rowY = 235.0f;

        for (int i = 0; i < n; ++i) {
            Rectangle r{ startX + i * (tileSz + gap), rowY, tileSz, tileSz };
            UI::DropShadow(r, 8.0f, 3.0f, 0.35f, 2.0f);
            if (i < (int)currentGuess.size()) {
                char c = (char)std::toupper((unsigned char)currentGuess[i]);
                std::string key = std::string("wheel/white_tiles/crossword_white_tile_") + c + ".png";
                UIAssets::drawFit(key, r);
            } else {
                UIAssets::drawFit("wheel/white_tiles/white_crossword_tile_blank.png", r);
            }
        }

        // Animated Astronaut with floating bobbing secondary motion & ground shadow
        float astroBob = std::sin(t * 2.2f) * 8.0f;
        DrawEllipse(435, 545, 64, 14, Theme::WithAlpha(BLACK, 0.35f));
        Rectangle astroRec{ 346, 336 + astroBob, 178, 212 };
        UIAssets::drawFrameSequence("wheel/astro_rgba/frame_", 16, t, 12.0f, astroRec);

        // Cosmic Wheel Base Disc
        DrawCircle(270, 735, 126.0f, Theme::WithAlpha(Color{ 15, 23, 42, 255 }, 0.70f));
        DrawCircleLines(270, 735, 126.0f, Theme::WithAlpha(Color{ 56, 189, 248, 255 }, 0.55f));
        DrawCircleLines(270, 735, 122.0f, Theme::WithAlpha(WHITE, 0.25f));

        // Live Word Preview Pill above the wheel
        if (!currentGuess.empty()) {
            Rectangle pill{ 160, 520, 220, 42 };
            UI::DropShadow(pill, 16.0f, 4.0f, 0.35f, 2.0f);
            UI::GradientRound(pill, 16.0f, Color{ 6, 182, 212, 240 }, Color{ 14, 116, 144, 240 });
            UI::Rim(pill, 16.0f, WHITE, WHITE, Color{ 165, 243, 252, 255 });
            Theme::text3D(currentGuess, { 270, 541 }, 22, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        // Connecting electric lines
        for (size_t i = 0; i + 1 < currentPath.size(); ++i) {
            Vector2 p1 = nodes[currentPath[i]].pos;
            Vector2 p2 = nodes[currentPath[i + 1]].pos;
            DrawLineEx(p1, p2, 12.0f, Color{ 6, 182, 212, 145 });
            DrawLineEx(p1, p2, 5.0f, Color{ 165, 243, 252, 255 });
        }

        // If dragging, draw trail from last selected node to mouse
        if (dragging && !currentPath.empty()) {
            Vector2 pLast = nodes[currentPath.back()].pos;
            DrawLineEx(pLast, m, 8.0f, Color{ 6, 182, 212, 110 });
            DrawLineEx(pLast, m, 3.0f, Color{ 165, 243, 252, 200 });
        }

        const char* wheelColors[7] = {
            "_red.png", "_green.png", "_yellow.png", "_blue.png",
            "_purple.png", "_orange.png", "_teal.png"
        };
        for (size_t i = 0; i < nodes.size(); ++i) {
            char c = nodes[i].letter;
            std::string lKey = std::string("wheel/wheel_letters/wheel_letter_") + c + wheelColors[(c - 'A') % 7];
            float sz = 66.0f * nodes[i].currentScale;
            Vector2 p = nodes[i].pos;
            DrawCircle((int)p.x, (int)(p.y + 4.0f * nodes[i].currentScale), sz * 0.44f, Theme::WithAlpha(BLACK, 0.40f));
            UIAssets::drawCentered(lKey, p, sz, sz);
        }

        // Bottom Action Buttons (Stateful Buttons)
        shuffleBtn.draw();
        hintBtn.draw();

        Theme::drawParticles();

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
