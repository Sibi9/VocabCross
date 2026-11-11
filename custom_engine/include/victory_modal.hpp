#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "word_card_modal.hpp"

#include <algorithm>
#include <cmath>
#include <string>

class VictoryModal {
public:
    bool active = false;
    float anim = 0.0f;
    bool nextClicked = false;
    bool mapClicked = false;
    WordData currentWord;
    WordCardModal wordCardModal;

    void init() {
        wordCardModal.init();
    }

    void show(const WordData& word) {
        currentWord = word;
        active = true;
        anim = 0.0f;
        nextClicked = false;
        mapClicked = false;
        wordCardModal.hide();
        Theme::coins += 50;
        gAudio.playFanfare();
        Theme::spawnCelebration({ 270.0f, 320.0f }, 80);
    }

    void hide() {
        active = false;
        wordCardModal.hide();
    }

    void update(float dt) {
        if (!active) return;
        anim = std::min(1.0f, anim + dt * 3.2f);
        nextClicked = false;
        mapClicked = false;

        if (wordCardModal.active) {
            wordCardModal.update(dt);
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            Rectangle nextRec{ 130, 825, 280, 82 };
            Rectangle replayRec{ 44, 845, 76, 76 };
            Rectangle cardBtnRec{ 110, 748, 320, 52 };
            Rectangle mascotRec{ 345, 815, 165, 145 };

            if (CheckCollisionPointRec(m, nextRec)) {
                gAudio.playClick();
                nextClicked = true;
            } else if (CheckCollisionPointRec(m, replayRec)) {
                gAudio.playClick();
                mapClicked = true;
            } else if (CheckCollisionPointRec(m, cardBtnRec)) {
                wordCardModal.show(currentWord);
            } else if (CheckCollisionPointRec(m, mascotRec)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 420.0f, 860.0f }, 48);
            }
        }
    }

    void draw(int w, int h) {
        if (!active) return;

        // 1. Clean Victory Scene Base (includes LEVEL COMPLETE!, 3 Golden Stars, EXCELLENT! Ribbon, Rewards Box, NEXT LEVEL & REPLAY buttons!)
        if (!UIAssets::drawStretch("victory/scene_base_without_mascot.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangle(0, 0, w, h, Color{ 8, 12, 26, 220 });
        }

        // 2. Solved English Word rendered dynamically with 3D Cream Alphabet Tiles (A-Z)
        UIAssets::drawWordTiles("image_guess/cream_tiles/tile_",
                                currentWord.word.empty() ? "GRADUATE" : currentWord.word,
                                { 270, 212 }, 430.0f, 46.0f, 4.0f);

        // 3. Arabic Translation Badge (Shaped RTL Arabic via Cairo.ttf)
        Rectangle arBadge{ 65, 678, 410, 58 };
        UI::DropShadow(arBadge, 18.0f, 4.0f, 0.35f, 2.0f);
        UI::GradientRound(arBadge, 18.0f, Color{ 15, 23, 42, 240 }, Color{ 30, 58, 138, 240 });
        UI::Rim(arBadge, 18.0f, Color{ 250, 204, 21, 255 }, Color{ 254, 240, 138, 255 }, Color{ 180, 83, 9, 255 });
        Theme::textArabic(currentWord.translation_ar.empty() ? "خريج / إتمام المرحلة بنجاح" : currentWord.translation_ar,
                          { 270, 707 }, 26, Color{ 254, 240, 138, 255 }, true, true, BLACK, 2.0f);

        // 4. Open Word Flashcard Button ("بطاقة الكلمة والمرادفات")
        Rectangle cardBtnRec{ 110, 748, 320, 50 };
        UI::GradientRound(cardBtnRec, 16.0f, Color{ 245, 158, 11, 255 }, Color{ 217, 119, 6, 255 });
        UI::Rim(cardBtnRec, 16.0f, Color{ 120, 53, 15, 255 }, Color{ 254, 243, 199, 255 }, Color{ 180, 83, 9, 255 });
        Theme::textArabic("بطاقة الكلمة التفصيلية (WORD CARD)", { 270, 773 }, 20, WHITE, true, true, BLACK, 2.0f);

        // 5. Animated Graduate Mascot + Floating Graduation Cap at bottom-right (matching ref8_victory_modal.jpg!)
        float bounceY = std::abs(std::sin(Theme::timeNow * 4.2f)) * -12.0f;
        float capLagY = std::sin(Theme::timeNow * 4.2f - 0.55f) * 8.0f - 6.0f;
        bool sparklePose = ((int)(Theme::timeNow * 2.5f) % 2) == 1;

        std::string bodyKey = sparklePose
            ? "victory/mascot_body_sparkle_aligned.png"
            : "victory/mascot_body_cheer_aligned.png";
        UIAssets::drawCentered(bodyKey, { 430, 895 + bounceY }, 145, 145);
        UIAssets::drawCentered("victory/graduation_cap_cropped.png",
                               { 432, 812 + bounceY + capLagY }, 105, 68,
                               std::sin(Theme::timeNow * 3.0f) * 4.5f);

        Theme::drawParticles();

        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
