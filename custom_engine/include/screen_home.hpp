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

    UI::StatefulButton playBtn;
    UI::StatefulButton dockBtns[4];
    UI::StatefulButton soundBtn;

    void init() {
        wordCardModal.init();

        // 1. Grand 3D Stateful PLAY Button
        playBtn.bounds = Rectangle{ 135, 715, 270, 78 };
        playBtn.label = "PLAY QUEST";
        playBtn.topColor = Color{ 34, 197, 94, 255 };
        playBtn.bottomColor = Color{ 21, 128, 61, 255 };
        playBtn.lipColor = Color{ 20, 83, 45, 255 };
        playBtn.borderColor = Color{ 254, 240, 138, 255 };
        playBtn.textColor = WHITE;
        playBtn.cornerRadius = 38.0f;
        playBtn.fontSize = 28.0f;

        // 2. Sound Toggle Stateful Button
        soundBtn.bounds = Rectangle{ 455, 18, 68, 42 };
        soundBtn.label = "SFX";
        soundBtn.topColor = Color{ 51, 65, 85, 230 };
        soundBtn.bottomColor = Color{ 30, 41, 59, 230 };
        soundBtn.lipColor = Color{ 15, 23, 42, 230 };
        soundBtn.borderColor = Color{ 148, 163, 184, 255 };
        soundBtn.cornerRadius = 12.0f;
        soundBtn.fontSize = 15.0f;

        // 3. Four Feature Dock Stateful Buttons
        dockBtns[0].bounds = Rectangle{ 10, 902, 124, 48 };
        dockBtns[0].label = "SHOP";
        dockBtns[0].iconKey = "hud/icon_shop_bulb.png";
        dockBtns[0].topColor = Color{ 124, 58, 237, 245 };
        dockBtns[0].bottomColor = Color{ 76, 29, 149, 245 };
        dockBtns[0].lipColor = Color{ 59, 7, 100, 245 };
        dockBtns[0].borderColor = Color{ 216, 180, 254, 255 };
        dockBtns[0].cornerRadius = 14.0f;
        dockBtns[0].fontSize = 14.0f;

        dockBtns[1].bounds = Rectangle{ 140, 902, 124, 48 };
        dockBtns[1].label = "STREAK";
        dockBtns[1].iconKey = "hud/icon_streak_fire.png";
        dockBtns[1].topColor = Color{ 234, 88, 12, 245 };
        dockBtns[1].bottomColor = Color{ 154, 52, 18, 245 };
        dockBtns[1].lipColor = Color{ 124, 45, 18, 245 };
        dockBtns[1].borderColor = Color{ 254, 215, 170, 255 };
        dockBtns[1].cornerRadius = 14.0f;
        dockBtns[1].fontSize = 14.0f;

        dockBtns[2].bounds = Rectangle{ 270, 902, 124, 48 };
        dockBtns[2].label = "ACADEMY";
        dockBtns[2].iconKey = "hud/icon_academy_castle.png";
        dockBtns[2].topColor = Color{ 37, 99, 235, 245 };
        dockBtns[2].bottomColor = Color{ 30, 58, 138, 245 };
        dockBtns[2].lipColor = Color{ 23, 37, 84, 245 };
        dockBtns[2].borderColor = Color{ 191, 219, 254, 255 };
        dockBtns[2].cornerRadius = 14.0f;
        dockBtns[2].fontSize = 14.0f;

        dockBtns[3].bounds = Rectangle{ 400, 902, 130, 48 };
        dockBtns[3].label = "WORD CARD";
        dockBtns[3].iconKey = "hud/icon_word_scale.png";
        dockBtns[3].topColor = Color{ 16, 185, 129, 245 };
        dockBtns[3].bottomColor = Color{ 6, 95, 70, 245 };
        dockBtns[3].lipColor = Color{ 4, 47, 46, 245 };
        dockBtns[3].borderColor = Color{ 167, 243, 208, 255 };
        dockBtns[3].cornerRadius = 14.0f;
        dockBtns[3].fontSize = 13.0f;
    }

    int update(float dt) {
        pendingAction = 0;
        if (wordCardModal.active) {
            wordCardModal.update(dt);
            return -1;
        }

        Vector2 m = Input::Mouse();
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        // Update Button States (Spring Scaling, Downward Press, Specular Shift)
        playBtn.update(m, mouseDown, dt);
        soundBtn.update(m, mouseDown, dt);
        for (int i = 0; i < 4; ++i) {
            dockBtns[i].update(m, mouseDown, dt);
        }

        // Keyboard shortcut: ENTER or SPACE starts Play immediately
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || playBtn.isClicked(m, mouseReleased)) {
            gAudio.playFanfare();
            Theme::spawnCelebration({ 270, 754 }, 64);
            pendingAction = 1;
            return Nav::SCREEN_MAP;
        }

        if (soundBtn.isClicked(m, mouseReleased)) {
            Theme::soundOn = !Theme::soundOn;
            soundBtn.label = Theme::soundOn ? "SFX" : "MUTE";
            gAudio.playClick();
            return -1;
        }

        if (dockBtns[0].isClicked(m, mouseReleased)) {
            gAudio.playPop();
            pendingAction = 2;
            return -1;
        }
        if (dockBtns[1].isClicked(m, mouseReleased)) {
            gAudio.playPop();
            pendingAction = 3;
            return -1;
        }
        if (dockBtns[2].isClicked(m, mouseReleased)) {
            gAudio.playPop();
            pendingAction = 4;
            return -1;
        }
        if (dockBtns[3].isClicked(m, mouseReleased)) {
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

        // Clicking Trio Mascots triggers interactive celebration
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, Rectangle{ 20, 560, 500, 140 })) {
            gAudio.playChime();
            Theme::spawnCelebration(m, 42);
        }

        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Layer 1 & 2: Distortion-Free 16-Frame Living Home Scene (Clean Backdrop & Atmospheric Animation)
        if (!UIAssets::drawFrameSequence("home_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 253, 230, 138, 255 });
        }

        // 2. Layer 3 & 4: 60-FPS Isolated RGBA Flying Magic Books with Dynamic Ground Shadows (Secondary Motion)
        struct BookDef {
            const char* key;
            Vector2 basePos;
            float freqX, ampX, freqY, ampY, rotSpeed;
            float shadowOffsetY;
        };
        BookDef books[3] = {
            { "home_props/flying_book_01_cream.png", { 56.0f, 414.0f }, 2.2f, 5.0f, 3.4f, 8.0f, 12.0f, 38.0f },
            { "home_props/flying_book_02_purple.png", { 362.0f, 442.0f }, 2.0f, -5.0f, 3.1f, 9.0f, -13.0f, 40.0f },
            { "home_props/flying_book_03_red.png", { 496.0f, 390.0f }, 2.5f, 4.0f, 3.6f, 7.0f, 11.0f, 36.0f }
        };

        for (int i = 0; i < 3; ++i) {
            float offsetY = std::sin(t * books[i].freqY) * books[i].ampY;
            float offsetX = std::cos(t * books[i].freqX) * books[i].ampX;
            Vector2 bPos{ books[i].basePos.x + offsetX, books[i].basePos.y + offsetY };
            float rot = std::sin(t * books[i].freqY) * books[i].rotSpeed;

            // Dynamic ground shadow that scales inversely with bob height
            float sScale = 1.0f - (offsetY / books[i].ampY) * 0.22f;
            DrawEllipse((int)bPos.x, (int)(books[i].basePos.y + books[i].shadowOffsetY),
                        24.0f * sScale, 6.0f * sScale, Theme::WithAlpha(BLACK, 0.24f * sScale));

            UIAssets::drawCentered(books[i].key, bPos, 66, 54, rot);
        }

        // 3. Layer 5: Stateful Grand PLAY Button (4-State Visual State Machine, Zero Circular Overlay)
        playBtn.draw();

        // 4. Layer 5: Sound Toggle Button
        soundBtn.draw();

        // 5. Layer 5: Sleek Bottom Feature Dock Buttons
        for (int i = 0; i < 4; ++i) {
            dockBtns[i].draw();
        }

        Theme::drawParticles();
        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
