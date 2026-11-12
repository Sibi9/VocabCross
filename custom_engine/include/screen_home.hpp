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

    struct HomePlayButton {
        Vector2 center{ 270.0f, 776.0f };
        Vector2 size{ 246.0f, 144.0f };
        float currentScale = 1.0f;
        float targetScale = 1.0f;
        float currentOffset = 0.0f;
        float targetOffset = 0.0f;
        float hoverGlow = 0.0f;
        bool isHovered = false;
        bool isPressed = false;

        void update(Vector2 mouse, bool mouseDown, float dt) {
            Rectangle hitArea{ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y };
            isHovered = CheckCollisionPointRec(mouse, hitArea);
            if (isHovered) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if (mouseDown) {
                    isPressed = true;
                    targetScale = 0.96f;
                    targetOffset = 3.5f;
                    hoverGlow = 0.8f;
                } else {
                    isPressed = false;
                    targetScale = 1.05f; // +5% smooth spring scale on hover
                    targetOffset = 0.0f;
                    hoverGlow = 1.0f;
                }
            } else {
                isPressed = false;
                targetScale = 1.0f;
                targetOffset = 0.0f;
                hoverGlow = 0.0f;
            }

            currentScale += (targetScale - currentScale) * std::min(1.0f, dt * 18.0f);
            currentOffset += (targetOffset - currentOffset) * std::min(1.0f, dt * 22.0f);
        }

        bool isClicked(Vector2 mouse, bool mouseReleased) const {
            Rectangle hitArea{ center.x - size.x * 0.5f, center.y - size.y * 0.5f, size.x, size.y };
            return mouseReleased && CheckCollisionPointRec(mouse, hitArea);
        }

        void draw() const {
            Vector2 pos{ center.x, center.y + currentOffset };
            float w = size.x * currentScale;
            float h = size.y * currentScale;

            // When hovered or pressed: draw the interactive spring-scaled button over the frame!
            // When idle, the living animation in the frame already renders the button naturally.
            if (currentScale > 1.005f || currentScale < 0.995f || std::abs(currentOffset) > 0.5f) {
                DrawEllipse((int)pos.x, (int)(pos.y + h * 0.42f), w * 0.45f, 14.0f * currentScale,
                            Theme::WithAlpha(BLACK, 0.35f));

                UIAssets::drawCentered("btn_play_emerald_gold_with_text.png", pos, w, h);

                if (hoverGlow > 0.05f) {
                    DrawEllipseLines((int)pos.x, (int)(pos.y - 2.0f), w * 0.46f, h * 0.40f,
                                     Theme::WithAlpha(Color{ 254, 240, 138, 255 }, hoverGlow * 0.65f));
                }
            }
        }
    };

    HomePlayButton playBtn;
    float soundScale = 1.0f;
    float settingsScale = 1.0f;
    float settingsRot = 0.0f;

    void init() {
        wordCardModal.init();
        pendingAction = 0;
        soundScale = 1.0f;
        settingsScale = 1.0f;
        settingsRot = 0.0f;
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

        // 1. Interactive 4-State Spring Play Button
        playBtn.update(m, mouseDown, dt);

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || playBtn.isClicked(m, mouseReleased)) {
            gAudio.playFanfare();
            Theme::spawnCelebration(playBtn.center, 64);
            pendingAction = 1;
            return Nav::SCREEN_MAP;
        }

        // 2. Sound Speaker Button at Top Right (414, 32, 50, 50)
        Rectangle soundRec{ 414, 32, 50, 50 };
        bool soundHov = CheckCollisionPointRec(m, soundRec);
        float targetSoundS = soundHov ? 1.08f : 1.0f;
        soundScale += (targetSoundS - soundScale) * std::min(1.0f, dt * 18.0f);
        if (soundHov) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (mouseReleased) {
                Theme::soundOn = !Theme::soundOn;
                gAudio.playClick();
                return -1;
            }
        }

        // 3. Settings Gear Button at Top Right (470, 32, 50, 50)
        Rectangle settingsRec{ 470, 32, 50, 50 };
        bool settingsHov = CheckCollisionPointRec(m, settingsRec);
        float targetSettingsS = settingsHov ? 1.08f : 1.0f;
        settingsScale += (targetSettingsS - settingsScale) * std::min(1.0f, dt * 18.0f);
        if (settingsHov) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            settingsRot += dt * 90.0f;
            if (mouseReleased) {
                gAudio.playClick();
                pendingAction = 2; // Open Booster Shop / Settings
                return -1;
            }
        }

        // 4. Interactive Avatar Lv.1 Badge at Top Left
        Rectangle avatarRec{ 16, 12, 75, 80 };
        if (CheckCollisionPointRec(m, avatarRec)) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gAudio.playPop();
                Theme::spawnCelebration({ 54, 52 }, 24);
            }
        }

        // 5. Interactive Heart Life Bar at Top Center
        Rectangle heartRec{ 115, 26, 250, 56 };
        if (CheckCollisionPointRec(m, heartRec)) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 240, 54 }, 32);
            }
        }

        // 6. Interactive Trio Mascots (Scholar Owl, Cheering Fox, Astronaut)
        Rectangle owlRec{ 16, 680, 145, 180 };
        Rectangle foxRec{ 375, 680, 145, 180 };
        Rectangle astroRec{ 195, 560, 150, 140 };

        if (CheckCollisionPointRec(m, owlRec)) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 88, 770 }, 36);
            }
        } else if (CheckCollisionPointRec(m, foxRec)) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 448, 770 }, 36);
            }
        } else if (CheckCollisionPointRec(m, astroRec)) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gAudio.playPop();
                Theme::spawnCelebration({ 270, 630 }, 36);
            }
        }

        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Distortion-Free 16-Frame Living Home Scene (Clean Atmosphere, Crest, Islands, Mascots & Play Button)
        if (!UIAssets::drawFrameSequence("home_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 253, 230, 138, 255 });
        }

        // 2. Interactive Sound / Mute feedback
        if (!Theme::soundOn) {
            DrawLineEx({ 422, 40 }, { 456, 74 }, 3.5f, Color{ 239, 68, 68, 245 });
            DrawLineEx({ 422, 40 }, { 456, 74 }, 1.5f, WHITE);
        }

        // 3. Settings Gear interactive hover rotation overlay
        if (settingsScale > 1.01f) {
            UIAssets::drawCentered("btn_settings_gear.png", { 495, 57 }, 50 * settingsScale, 50 * settingsScale, settingsRot);
        }

        // 4. Stateful Grand PLAY Button (4-State Spring Scale, Zero Circular Overlays, Zero Duplicate Text)
        playBtn.draw();

        Theme::drawParticles();

        if (wordCardModal.active) {
            wordCardModal.draw(w, h);
        }
    }
};
