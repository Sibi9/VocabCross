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
            Rectangle gotItRec{ 140, 818, 260, 74 };
            Rectangle audioRec{ 412, 132, 58, 58 };
            Rectangle noteRec{ 62, 818, 68, 74 };

            if (CheckCollisionPointRec(m, gotItRec)) {
                gAudio.playClick();
                hide();
            } else if (CheckCollisionPointRec(m, audioRec)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 441, 161 }, 24);
            } else if (CheckCollisionPointRec(m, noteRec)) {
                savedToNotebook = !savedToNotebook;
                gAudio.playPop();
                Theme::spawnCelebration({ 96, 855 }, 20);
            }
        }
    }

    void draw(int w, int h) {
        if (!active) return;

        DrawRectangle(0, 0, w, h, Color{ 8, 14, 28, (unsigned char)(215 * anim) });

        Rectangle cardRec{ 22, 42, 496, 885 };
        UIAssets::drawStretch("word_card/ui_modal_card_blank.png", cardRec);

        // Top "NEW WORD!" Badge
        UIAssets::drawFit("word_card/header_badge_new_word.png", Rectangle{ 145, 6, 250, 96 });

        // 3D Cyan Alphabet Tiles spelling the word dynamically + Speaker Icon
        UIAssets::drawWordTiles("word_card/cyan_letters/letter_", currentWord.word, { 242, 152 }, 335.0f, 42.0f, 3.0f);
        UIAssets::drawFit("word_card/btn_audio_speaker.png", Rectangle{ 412, 126, 56, 52 });

        // Central Golden Scale Illustration
        float bob = std::sin(Theme::timeNow * 3.2f) * 4.0f;
        UIAssets::drawFit("word_card/icon_integrity_scale_hd.png", Rectangle{ 185, 195 + bob, 170, 145 });

        // Arabic Translation Banner (Shaped RTL Arabic via Cairo.ttf)
        Rectangle arabBox{ 56, 350, 428, 68 };
        UIAssets::drawStretch("word_card/ui_section_box_blank.png", arabBox);
        Theme::textArabic(currentWord.translation_ar.empty() ? "النزاهة والاستقامة" : currentWord.translation_ar,
                          { 270, 384 }, 28, Color{ 30, 58, 138, 255 }, true, true, WHITE, 2.0f);

        // Section 1: Part of Speech & Synonym
        Rectangle sec1{ 56, 430, 428, 92 };
        UIAssets::drawStretch("word_card/ui_section_box_blank.png", sec1);
        Theme::text3D("PART OF SPEECH & SYNONYM:", { 76, 444 }, 15, Color{ 100, 116, 139, 255 }, WHITE, 1.0f, BLANK, false, false);
        Theme::text3D("Noun  |  " + (currentWord.synonym_en.empty() ? "Honesty, Probity" : currentWord.synonym_en),
                      { 76, 474 }, 18, Color{ 37, 99, 235, 255 }, WHITE, 1.0f, BLANK, false, false);

        // Section 2: Curriculum Definition
        Rectangle sec2{ 56, 534, 428, 122 };
        UIAssets::drawStretch("word_card/ui_section_box_blank.png", sec2);
        Theme::text3D("DEFINITION:", { 76, 548 }, 15, Color{ 100, 116, 139, 255 }, WHITE, 1.0f, BLANK, false, false);
        UIAssets::drawWrappedText3D(
            currentWord.definition_en.empty()
                ? "The quality of being honest and having strong moral principles."
                : currentWord.definition_en,
            Rectangle{ 72, 572, 396, 74 }, 17, Color{ 15, 23, 42, 255 }, WHITE, 1.0f, false
        );

        // Section 3: Contextual BAC Example
        Rectangle sec3{ 56, 668, 428, 122 };
        UIAssets::drawStretch("word_card/ui_section_box_blank.png", sec3);
        Theme::text3D("EXAMPLE:", { 76, 682 }, 15, Color{ 100, 116, 139, 255 }, WHITE, 1.0f, BLANK, false, false);
        UIAssets::drawWrappedText3D(
            "She showed academic " + currentWord.word + " by refusing to cheat on the exam.",
            Rectangle{ 72, 706, 396, 74 }, 17, Color{ 15, 23, 42, 255 }, WHITE, 1.0f, false
        );

        // Bottom Notebook Button & GOT IT! Button
        UIAssets::drawFit("word_card/btn_notebook_blue.png", Rectangle{ 58, 818, 76, 74 });
        UIAssets::drawFit("word_card/btn_got_it_orange.png", Rectangle{ 146, 818, 336, 74 });
    }
};
