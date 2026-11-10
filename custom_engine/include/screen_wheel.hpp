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
