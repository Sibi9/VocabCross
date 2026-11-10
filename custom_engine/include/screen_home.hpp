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

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();

            // Sound toggle button
            if (CheckCollisionPointRec(m, Rectangle{ 412, 16, 50, 50 })) {
                Theme::soundOn = !Theme::soundOn;
                gAudio.playClick();
                return -1;
            }
            // Main PLAY button
            if (CheckCollisionPointRec(m, Rectangle{ 115, 735, 310, 96 })) {
                gAudio.playFanfare();
                Theme::spawnCelebration({ 270, 780 }, 56);
                pendingAction = 1;
                return Nav::SCREEN_MAP;
            }
            // Bottom Dock: 4 Quick Feature Buttons
            // 1. Booster Shop
            if (CheckCollisionPointRec(m, Rectangle{ 18, 852, 118, 86 })) {
                gAudio.playPop();
                pendingAction = 2;
                return -1;
            }
            // 2. Daily Streak (1-7)
            if (CheckCollisionPointRec(m, Rectangle{ 146, 852, 118, 86 })) {
                gAudio.playPop();
                pendingAction = 3;
                return -1;
            }
            // 3. Graduation Academy Map (1-14)
            if (CheckCollisionPointRec(m, Rectangle{ 274, 852, 118, 86 })) {
                gAudio.playPop();
                pendingAction = 4;
                return -1;
            }
            // 4. Word Card ("INTEGRITY")
            if (CheckCollisionPointRec(m, Rectangle{ 402, 852, 120, 86 })) {
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
            if (CheckCollisionPointRec(m, Rectangle{ 50, 410, 440, 290 })) {
                gAudio.playChime();
                Theme::spawnCelebration(m, 42);
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Living Sky Gradient Backdrop + Soft Clouds
        DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 224, 242, 254, 255 });
        DrawRectangleGradientV(0, h / 2, w, h / 2, Color{ 224, 242, 254, 255 }, Color{ 167, 243, 208, 255 });

        for (int i = 0; i < 5; ++i) {
            float cx = std::fmod(i * 145.0f + t * (12.0f + i * 3.0f), 680.0f) - 70.0f;
            float cy = 95.0f + (i % 3) * 110.0f;
            DrawCircle((int)cx, (int)cy, 38.0f, Color{ 255, 255, 255, 140 });
            DrawCircle((int)(cx + 28), (int)(cy + 6), 28.0f, Color{ 255, 255, 255, 140 });
            DrawCircle((int)(cx - 26), (int)(cy + 8), 26.0f, Color{ 255, 255, 255, 140 });
        }

        // 2. Floating Islands & Magic Books (from bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books)
        float isl1Y = std::sin(t * 2.1f) * 8.0f;
        float isl2Y = std::sin(t * 1.8f + 1.4f) * 9.0f;
        float isl3Y = std::sin(t * 2.4f + 2.8f) * 7.0f;

        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/island_01_observatory_telescope.png",
                               { 92, 250 + isl1Y }, 155, 155);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/island_02_classical_greek_temple.png",
                               { 448, 242 + isl2Y }, 155, 155);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/island_03_open_book_fairytale_castle.png",
                               { 270, 560 + isl3Y * 0.4f }, 480, 290);

        // Fluttering Magic Books
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/flying_book_01_cream.png",
                               { 68, 395 + std::sin(t * 3.8f) * 10.0f }, 76, 76, std::sin(t * 4.0f) * 8.0f);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/flying_book_02_purple.png",
                               { 472, 385 + std::cos(t * 3.4f) * 11.0f }, 76, 76, std::cos(t * 3.5f) * -8.0f);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/03_floating_islands_and_magic_books/flying_book_03_red.png",
                               { 440, 128 + std::sin(t * 4.2f + 1.0f) * 8.0f }, 68, 68, std::sin(t * 3.2f) * 6.0f);

        // 3. Top HUD Bar (from bac_vocab_quest_ui_kit/01_hud_and_emblem_crest)
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/hud_avatar_lv1_badge.png",
                          Rectangle{ 14, 12, 165, 58 });
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/hud_heart_life_bar.png",
                          Rectangle{ 192, 16, 145, 50 });
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/btn_sound_speaker.png",
                          Rectangle{ 412, 16, 50, 50 });
        UIAssets::drawFit("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/btn_settings_gear.png",
                          Rectangle{ 472, 16, 50, 50 });

        // 4. Center Emblem Crest "BAC VOCAB QUEST" with Breathing Scale
        float crestScale = 1.0f + 0.03f * std::sin(t * 2.6f);
        if (!UIAssets::drawCentered("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/emblem_bac_vocab_quest_crest.png",
                                   { 270, 225 }, 350 * crestScale, 255 * crestScale)) {
            Rectangle embRec{ 270 - 160 * crestScale, 225 - 60 * crestScale, 320 * crestScale, 120 * crestScale };
            UI::DropShadow(embRec, 20.0f, 6.0f, 0.4f, 2.0f);
            UI::GradientRound(embRec, 22.0f, Color{ 30, 58, 138, 255 }, Color{ 15, 23, 42, 255 });
            UI::Rim(embRec, 22.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::text3D("BAC VOCAB QUEST", { 270, 225 }, 28 * crestScale, Color{ 253, 224, 71, 255 }, Color{ 120, 53, 15, 255 }, 3.0f, BLACK, true, true);
        }

        // 5. 3D Gold-Blue BAC Alphabet Banner ("BAC QUEST")
        UIAssets::drawWordTiles("04_bac_gold_blue_alphabet_a_to_z/bac_letter_",
                                "BACQUEST", { 270, 382 }, 390.0f, 44.0f, 3.0f);

        // 6. Animated Trio Mascots (Scholar Owl, Astronaut, Detective Fox)
        // Left: Scholar Owl (Body + Waving Wing)
        float owlBob = std::sin(t * 3.0f) * 6.0f;
        float wingRot = std::sin(t * 5.2f) * 16.0f;
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/scholar_owl_waving_wing.png",
                               { 158, 545 + owlBob }, 85, 85, wingRot);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/scholar_owl_body_clean.png",
                               { 122, 548 + owlBob }, 168, 185);

        // Right: Cheering Detective Fox
        float foxBob = std::abs(std::sin(t * 3.6f + 0.8f)) * -10.0f;
        float foxTilt = std::sin(t * 3.6f) * 4.5f;
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/cheering_fox_full_rgba.png",
                               { 415, 548 + foxBob }, 172, 188, foxTilt);

        // Center Hero: Celebrating Astronaut (Body + Articulated Left/Right Arms)
        float astroBob = std::sin(t * 2.8f + 0.4f) * 8.0f;
        float armSwing = std::sin(t * 4.8f) * 18.0f;
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/astronaut_left_arm.png",
                               { 212, 495 + astroBob }, 82, 82, -armSwing);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/astronaut_right_arm.png",
                               { 328, 495 + astroBob }, 82, 82, armSwing);
        UIAssets::drawCentered("bac_vocab_quest_ui_kit/06_trio_spritesheets_and_layers/astronaut_body_armless.png",
                               { 270, 520 + astroBob }, 190, 215);

        // 7. Emerald Play Tiles Preview ("PLAY") & Giant Emerald-Gold CTA Button
        float btnPulse = 1.0f + 0.04f * std::sin(t * 3.8f);
        if (!UIAssets::drawCentered("bac_vocab_quest_ui_kit/01_hud_and_emblem_crest/btn_play_emerald_gold_with_text.png",
                                   { 270, 782 }, 320 * btnPulse, 98 * btnPulse)) {
            Rectangle pRec{ 270 - 150 * btnPulse, 782 - 44 * btnPulse, 300 * btnPulse, 88 * btnPulse };
            UI::DropShadow(pRec, 24.0f, 6.0f, 0.45f, 3.0f);
            UI::GradientRound(pRec, 24.0f, Color{ 34, 197, 94, 255 }, Color{ 21, 128, 61, 255 });
            UI::Rim(pRec, 24.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 20, 83, 45, 255 });
            Theme::text3D("PLAY", { 270, 782 }, 42 * btnPulse, WHITE, Color{ 20, 83, 45, 255 }, 3.5f, BLACK, true, true);
        }

        // 8. Bottom Feature Dock (Shop, Daily Streak 1-7, Graduation Academy 1-14, Word Card INTEGRITY)
        struct DockBtn {
            Rectangle r;
            const char* iconKey;
            const char* labelEn;
            const char* labelAr;
            Color topCol;
            Color botCol;
        };
        DockBtn dock[4] = {
            { { 18, 852, 118, 86 }, "magical_alchemist_booster_shop_ui_kit/03_booster_icons_hd/icon_magic_lightbulb_hd.png",
              "SHOP", "المتجر", Color{ 124, 58, 237, 255 }, Color{ 76, 29, 149, 255 } },
            { { 146, 852, 118, 86 }, "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/icon_streak_fire_hd.png",
              "STREAK 1-7", "المثابرة", Color{ 234, 88, 12, 255 }, Color{ 154, 52, 18, 255 } },
            { { 274, 852, 118, 86 }, "image2_graduation_academy_map_assets/building_big_blue_castle.png",
              "ACADEMY", "الأكاديمية", Color{ 37, 99, 235, 255 }, Color{ 30, 58, 138, 255 } },
            { { 402, 852, 120, 86 }, "image1_integrity_word_card_ui_kit/icon_integrity_scale_hd.png",
              "WORD CARD", "بطاقة كلمة", Color{ 16, 185, 129, 255 }, Color{ 6, 95, 70, 255 } }
        };

        for (int i = 0; i < 4; ++i) {
            UI::DropShadow(dock[i].r, 18.0f, 4.0f, 0.35f, 2.0f);
            UI::GradientRound(dock[i].r, 18.0f, dock[i].topCol, dock[i].botCol);
            UI::Rim(dock[i].r, 18.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            UIAssets::drawCentered(dock[i].iconKey, { dock[i].r.x + dock[i].r.width * 0.5f, dock[i].r.y + 30.0f }, 44, 44);
            Theme::text3D(dock[i].labelEn, { dock[i].r.x + dock[i].r.width * 0.5f, dock[i].r.y + 66.0f },
                          14, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        Theme::drawParticles();
        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
