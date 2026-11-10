#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <string>

class WordCardModal {
public:
    bool active = false;
    float anim = 0.0f;
    bool savedToNotebook = false;
    WordData currentWord;

    void init() {
        currentWord.word = "INTEGRITY";
        currentWord.translation_ar = "النزاهة / الاستقامة والأمانة";
        currentWord.definition_en = "The quality of being honest and having strong moral principles.";
        currentWord.synonym_en = "Honesty, Probity, Rectitude";
        currentWord.frequency = "High Frequency - Ethics in Business";
    }

    void show(const WordData& w) {
        if (!w.word.empty()) {
            currentWord = w;
        } else {
            init();
        }
        active = true;
        anim = 0.0f;
        gAudio.playPop();
    }

    void hide() {
        active = false;
    }

    void update(float dt) {
        if (!active) return;
        anim = std::min(1.0f, anim + dt * 4.0f);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            Rectangle gotItRec{ 130, 812, 280, 74 };
            Rectangle audioRec{ 415, 215, 58, 58 };
            Rectangle noteRec{ 68, 215, 58, 58 };

            if (CheckCollisionPointRec(m, gotItRec)) {
                gAudio.playClick();
                hide();
            } else if (CheckCollisionPointRec(m, audioRec)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 444, 244 }, 24);
            } else if (CheckCollisionPointRec(m, noteRec)) {
                savedToNotebook = !savedToNotebook;
                gAudio.playPop();
                Theme::spawnCelebration({ 97, 244 }, 20);
            }
        }
    }

    void draw(int w, int h) {
        if (!active) return;

        DrawRectangle(0, 0, w, h, Color{ 8, 14, 28, (unsigned char)(215 * anim) });

        float scale = 0.88f + 0.12f * anim;
        Rectangle cardRec{
            270.0f - 245.0f * scale,
            485.0f - 420.0f * scale,
            490.0f * scale,
            840.0f * scale
        };

        // 1. Main Blank Card Container from image1_integrity_word_card_ui_kit
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_modal_card_blank.png", cardRec)) {
            UI::DropShadow(cardRec, 28.0f, 10.0f, 0.45f, 4.0f);
            UI::GradientRound(cardRec, 28.0f, Color{ 248, 250, 255, 255 }, Color{ 225, 236, 250, 255 });
            UI::Rim(cardRec, 28.0f, Color{ 245, 158, 11, 255 }, WHITE, Color{ 180, 130, 40, 255 });
        }

        // 2. Top "NEW WORD DISCOVERED" Badge
        Rectangle badgeRec{ 115, 56, 310, 68 };
        if (!UIAssets::drawFit("image1_integrity_word_card_ui_kit/header_badge_new_word.png", badgeRec)) {
            UI::GradientRound(badgeRec, 20.0f, Color{ 251, 191, 36, 255 }, Color{ 217, 119, 6, 255 });
            Theme::text3D("NEW WORD CARD", { 270, 90 }, 24, WHITE, Color{ 120, 53, 15, 255 }, 2.5f, BLACK, true, true);
        }

        // 3. 3D Alphabet Tiles (A-Z) spelling the word dynamically
        Vector2 wordCenter{ 270, 162 };
        if (!UIAssets::drawWordTiles("alphabet_tiles_a_to_z/letter_", currentWord.word, wordCenter, 420.0f, 48.0f, 4.0f)) {
            Theme::text3D(currentWord.word, wordCenter, 36, Color{ 30, 64, 175, 255 }, WHITE, 3.0f, BLACK, true, true);
        }

        // 4. Audio Speaker & Notebook Buttons + Central Illustration (Golden Scale / Concept Icon)
        Rectangle noteRec{ 68, 215, 58, 58 };
        Rectangle audioRec{ 415, 215, 58, 58 };
        UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_notebook_blue.png", noteRec);
        UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_audio_speaker.png", audioRec);

        float bob = std::sin(Theme::timeNow * 3.2f) * 5.0f;
        Rectangle iconRec{ 185, 200 + bob, 170, 155 };
        if (!UIAssets::drawFit("image1_integrity_word_card_ui_kit/icon_integrity_scale_hd.png", iconRec)) {
            UIAssets::drawFit("icon_integrity_scale_original.png", iconRec);
        }

        // 5. Arabic Translation Banner (Shaped RTL Arabic via Cairo.ttf)
        Rectangle arabBox{ 58, 362, 424, 72 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", arabBox)) {
            UI::GradientRound(arabBox, 16.0f, Color{ 254, 243, 199, 255 }, Color{ 253, 230, 138, 255 });
        }
        Theme::textArabic(currentWord.translation_ar.empty() ? "النزاهة والاستقامة" : currentWord.translation_ar,
                          { 270, 398 }, 30, Color{ 15, 23, 42, 255 }, true, true, WHITE, 2.0f);

        // 6. Section 1: Part of Speech & BAC Unit Frequency
        Rectangle sec1{ 58, 446, 424, 96 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", sec1)) {
            UI::GradientRound(sec1, 16.0f, WHITE, Color{ 241, 245, 249, 255 });
        }
        Theme::text3D("BAC STREAM & SYNONYMS:", { 80, 462 }, 17, Color{ 37, 99, 235, 255 }, WHITE, 1.5f, BLANK, false, false);
        Theme::text("Synonym: " + (currentWord.synonym_en.empty() ? "Honesty / Probity" : currentWord.synonym_en),
                    { 80, 490 }, 18, Color{ 30, 41, 59, 255 }, false, false);
        Theme::text("BAC Priority: " + (currentWord.frequency.empty() ? "High Exam Frequency" : currentWord.frequency),
                    { 80, 514 }, 16, Color{ 180, 83, 9, 255 }, false, false);

        // 7. Section 2: Curriculum Definition
        Rectangle sec2{ 58, 554, 424, 118 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", sec2)) {
            UI::GradientRound(sec2, 16.0f, WHITE, Color{ 241, 245, 249, 255 });
        }
        Theme::text3D("DEFINITION:", { 80, 570 }, 18, Color{ 217, 119, 6, 255 }, WHITE, 1.5f, BLANK, false, false);
        Theme::text(currentWord.definition_en.empty()
                        ? "The quality of being honest and having strong moral principles."
                        : currentWord.definition_en,
                    { 80, 602 }, 17, Color{ 15, 23, 42, 255 }, false, false);

        // 8. Section 3: Contextual BAC Example
        Rectangle sec3{ 58, 684, 424, 110 };
        if (!UIAssets::drawStretch("image1_integrity_word_card_ui_kit/ui_section_box_blank.png", sec3)) {
            UI::GradientRound(sec3, 16.0f, WHITE, Color{ 241, 245, 249, 255 });
        }
        Theme::text3D("BAC CONTEXTUAL EXAMPLE:", { 80, 700 }, 17, Color{ 22, 163, 74, 255 }, WHITE, 1.5f, BLANK, false, false);
        Theme::text("Business leaders must act with " + currentWord.word + " and transparency.",
                    { 80, 730 }, 16, Color{ 51, 65, 85, 255 }, false, false);

        // 9. Orange CTA Button ("GOT IT!")
        Rectangle gotItRec{ 130, 812, 280, 74 };
        if (!UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_got_it_orange.png", gotItRec)) {
            UIAssets::drawFit("image1_integrity_word_card_ui_kit/btn_cta_orange_blank.png", gotItRec);
            Theme::text3D("GOT IT!", { 270, 849 }, 28, WHITE, Color{ 154, 52, 18, 255 }, 3.0f, BLACK, true, true);
        }
    }
};
