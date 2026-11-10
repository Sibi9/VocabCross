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
            Rectangle nextRec{ 170, 770, 290, 82 };
            Rectangle replayRec{ 72, 772, 82, 82 };
            Rectangle cardBtnRec{ 130, 695, 280, 56 };
            Rectangle mascotRec{ 170, 410, 200, 210 };

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
                Theme::spawnCelebration({ 270.0f, 470.0f }, 48);
            }
        }
    }

    void draw(int w, int h) {
        if (!active) return;

        // 1. Blurred Board Backdrop + Dark Overlay
        if (!UIAssets::drawStretch("word_graduate_victory_ui_kit/ui_components/bg_blurred_word_board.png",
                                   Rectangle{ 0, 0, (float)w, (float)h },
                                   Color{ 255, 255, 255, (unsigned char)(220 * anim) })) {
            DrawRectangle(0, 0, w, h, Color{ 8, 12, 26, (unsigned char)(210 * anim) });
        } else {
            DrawRectangle(0, 0, w, h, Color{ 8, 12, 26, (unsigned char)(115 * anim) });
        }

        // 2. Main Modal Card Container
        Rectangle modalRec{ 28, 70, 484, 820 };
        if (!UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/scene_base_without_mascot.png", modalRec)) {
            UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/modal_victory_card_full.png", modalRec);
        }

        // 3. Top Header "LEVEL COMPLETE!" & 3 Golden Stars with Pulse
        Rectangle headerRec{ 65, 58, 410, 105 };
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/header_level_complete.png", headerRec);

        float starPulse = 1.0f + 0.06f * std::sin(Theme::timeNow * 4.5f);
        UIAssets::drawCentered("word_graduate_victory_ui_kit/rewards_and_stars/three_golden_stars.png",
                               { 270, 188 }, 280 * starPulse, 110 * starPulse);

        // 4. Ribbon Banner ("EXCELLENT!")
        Rectangle ribbonRec{ 110, 238, 320, 68 };
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/ribbon_excellent_banner.png", ribbonRec);

        // 5. Solved English Word rendered with 3D Victory Alphabet Tiles (A-Z)
        UIAssets::drawWordTiles("word_graduate_victory_ui_kit/letter_tiles_a_z/tile_",
                                currentWord.word.empty() ? "GRADUATE" : currentWord.word,
                                { 270, 325 }, 420.0f, 52.0f, 4.0f);

        // 6. Arabic Translation Badge (Shaped RTL Arabic via Cairo.ttf)
        Rectangle arBadge{ 76, 362, 388, 56 };
        UI::GradientRound(arBadge, 18.0f, Color{ 15, 23, 42, 235 }, Color{ 30, 58, 138, 235 });
        UI::Rim(arBadge, 18.0f, Color{ 250, 204, 21, 255 }, Color{ 254, 240, 138, 255 }, Color{ 180, 83, 9, 255 });
        Theme::textArabic(currentWord.translation_ar.empty() ? "خريج / إتمام المرحلة بنجاح" : currentWord.translation_ar,
                          { 270, 390 }, 28, Color{ 254, 240, 138, 255 }, true, true, BLACK, 2.0f);

        // 7. Animated Graduate Mascot + Floating Graduation Cap + Floating Rewards
        float bounceY = std::abs(std::sin(Theme::timeNow * 4.2f)) * -14.0f;
        float capLagY = std::sin(Theme::timeNow * 4.2f - 0.55f) * 10.0f - 8.0f;
        bool sparklePose = ((int)(Theme::timeNow * 2.5f) % 2) == 1;

        std::string bodyKey = sparklePose
            ? "word_graduate_victory_ui_kit/mascot_layers/mascot_body_sparkle_aligned.png"
            : "word_graduate_victory_ui_kit/mascot_layers/mascot_body_cheer_aligned.png";
        UIAssets::drawCentered(bodyKey, { 270, 528 + bounceY }, 195, 195);
        UIAssets::drawCentered("word_graduate_victory_ui_kit/mascot_layers/graduation_cap_cropped.png",
                               { 270, 426 + bounceY + capLagY }, 135, 78,
                               std::sin(Theme::timeNow * 3.0f) * 4.5f);

        // Floating Coins & Magic XP Potion
        float floatL = std::sin(Theme::timeNow * 3.0f) * 6.0f;
        float floatR = std::cos(Theme::timeNow * 3.0f) * 6.0f;
        UIAssets::drawCentered("word_graduate_victory_ui_kit/rewards_and_stars/reward_coins_stack.png",
                               { 108, 545 + floatL }, 88, 88);
        Theme::text3D("+50", { 108, 598 + floatL }, 22, Color{ 253, 224, 71, 255 }, BLACK, 2.5f, BLACK, true, true);

        UIAssets::drawCentered("word_graduate_victory_ui_kit/rewards_and_stars/reward_magic_xp_potion.png",
                               { 432, 545 + floatR }, 88, 88);
        Theme::text3D("+25 XP", { 432, 598 + floatR }, 20, Color{ 103, 232, 249, 255 }, BLACK, 2.5f, BLACK, true, true);

        // 8. Open Word Flashcard Button ("بطاقة الكلمة التفصيلية")
        Rectangle cardBtnRec{ 130, 695, 280, 56 };
        UI::GradientRound(cardBtnRec, 18.0f, Color{ 245, 158, 11, 255 }, Color{ 217, 119, 6, 255 });
        UI::Rim(cardBtnRec, 18.0f, Color{ 120, 53, 15, 255 }, Color{ 254, 243, 199, 255 }, Color{ 180, 83, 9, 255 });
        Theme::textArabic("بطاقة الكلمة والمرادفات (WORD CARD)", { 270, 723 }, 21, WHITE, true, true, BLACK, 2.0f);

        // 9. Replay / Map Circle Button & Next Level CTA Button
        Rectangle replayRec{ 72, 772, 82, 82 };
        Rectangle nextRec{ 170, 770, 290, 82 };
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/btn_replay_circle.png", replayRec);
        UIAssets::drawFit("word_graduate_victory_ui_kit/ui_components/btn_next_level_cta.png", nextRec);

        Theme::drawParticles();

        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
