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

        // Keyboard shortcut: ENTER or SPACE starts Play immediately
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            gAudio.playFanfare();
            pendingAction = 1;
            return Nav::SCREEN_MAP;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();

            // Sound toggle button (top right)
            if (CheckCollisionPointRec(m, Rectangle{ 398, 16, 62, 62 })) {
                Theme::soundOn = !Theme::soundOn;
                gAudio.playClick();
                return -1;
            }
            // Main PLAY button (generous hitbox covering the big green PLAY button)
            if (CheckCollisionPointRec(m, Rectangle{ 135, 695, 270, 160 })) {
                gAudio.playFanfare();
                Theme::spawnCelebration({ 270, 775 }, 56);
                pendingAction = 1;
                return Nav::SCREEN_MAP;
            }
            // Compact Bottom Feature Dock (4 Quick Feature Pills at y=898..954)
            if (CheckCollisionPointRec(m, Rectangle{ 10, 898, 124, 54 })) {
                gAudio.playPop();
                pendingAction = 2;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 140, 898, 124, 54 })) {
                gAudio.playPop();
                pendingAction = 3;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 270, 898, 124, 54 })) {
                gAudio.playPop();
                pendingAction = 4;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 400, 898, 130, 54 })) {
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
            if (CheckCollisionPointRec(m, Rectangle{ 20, 560, 500, 320 })) {
                gAudio.playChime();
                Theme::spawnCelebration(m, 42);
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;
        Vector2 m = Input::Mouse();

        // 1. Distortion-Free 16-Frame Living Home Screen
        if (!UIAssets::drawFrameSequence("home_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 253, 230, 138, 255 });
        }

        // 1B. 60-FPS Isolated RGBA Flying Magic Books (Zero background distortion!)
        UIAssets::drawCentered("home_props/flying_book_01_cream.png",
                               { 56.0f + std::cos(t * 2.2f) * 5.0f, 414.0f + std::sin(t * 3.4f) * 8.0f },
                               64, 52, std::sin(t * 3.4f) * 12.0f);
        UIAssets::drawCentered("home_props/flying_book_02_purple.png",
                               { 362.0f - std::sin(t * 2.0f) * 5.0f, 442.0f + std::cos(t * 3.1f) * 9.0f },
                               68, 56, std::cos(t * 3.1f) * -13.0f);
        UIAssets::drawCentered("home_props/flying_book_03_red.png",
                               { 496.0f + std::sin(t * 2.5f) * 4.0f, 390.0f + std::sin(t * 3.6f + 1.2f) * 7.0f },
                               64, 52, std::sin(t * 3.0f) * 11.0f);


        // 2. Interactive Hover Glow on PLAY button when cursor is over it
        Rectangle playHit{ 148, 708, 244, 134 };
        if (CheckCollisionPointRec(m, playHit)) {
            UI::Rim(playHit, 42.0f, Color{ 254, 240, 138, 220 }, WHITE, Color{ 250, 204, 21, 180 });
        }

        // 3. Sleek Bottom Feature Dock with Hover Scaling
        struct DockBtn {
            Rectangle r;
            const char* iconKey;
            const char* labelEn;
            Color topCol;
            Color botCol;
        };
        DockBtn dock[4] = {
            { { 12, 902, 122, 48 }, "hud/icon_shop_bulb.png",
              "SHOP", Color{ 124, 58, 237, 245 }, Color{ 76, 29, 149, 245 } },
            { { 142, 902, 122, 48 }, "hud/icon_streak_fire.png",
              "STREAK", Color{ 234, 88, 12, 245 }, Color{ 154, 52, 18, 245 } },
            { { 272, 902, 122, 48 }, "hud/icon_academy_castle.png",
              "ACADEMY", Color{ 37, 99, 235, 245 }, Color{ 30, 58, 138, 245 } },
            { { 402, 902, 126, 48 }, "hud/icon_word_scale.png",
              "WORD CARD", Color{ 16, 185, 129, 245 }, Color{ 6, 95, 70, 245 } }
        };

        for (int i = 0; i < 4; ++i) {
            bool hovered = CheckCollisionPointRec(m, dock[i].r);
            Rectangle drawR = dock[i].r;
            if (hovered) {
                drawR.y -= 3.0f;
            }
            UI::DropShadow(drawR, 14.0f, 3.0f, 0.35f, 2.0f);
            UI::GradientRound(drawR, 14.0f, dock[i].topCol, dock[i].botCol);
            UI::Rim(drawR, 14.0f, hovered ? WHITE : Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            UIAssets::drawCentered(dock[i].iconKey, { drawR.x + 22.0f, drawR.y + 24.0f }, 30, 30);
            Theme::text3D(dock[i].labelEn, { drawR.x + 76.0f, drawR.y + 24.0f },
                          13, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        Theme::drawParticles();
        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
