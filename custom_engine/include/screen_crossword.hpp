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
        DrawRectangleRoundedLines(glowRec, 0.22f, 8, 4.0f, Color{ 253, 224, 71, 235 });

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
