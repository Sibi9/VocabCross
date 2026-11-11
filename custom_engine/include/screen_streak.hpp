#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <cmath>
#include <string>

class ScreenStreak {
public:
    int currentStreakDay = 4;
    bool rewardClaimedToday = false;

    UI::StatefulButton closeBtn;
    UI::StatefulButton claimBtn;

    void init() {
        currentStreakDay = gGameData.streak_day;
        rewardClaimedToday = false;

        closeBtn.bounds = Rectangle{ 458.0f, 14.0f, 70.0f, 42.0f };
        closeBtn.label = "CLOSE";
        closeBtn.cornerRadius = 12.0f;
        closeBtn.fontSize = 13.0f;
        closeBtn.topColor = Color{ 239, 68, 68, 245 };
        closeBtn.bottomColor = Color{ 185, 28, 28, 245 };
        closeBtn.lipColor = Color{ 153, 27, 27, 245 };
        closeBtn.borderColor = Color{ 254, 202, 202, 255 };

        claimBtn.bounds = Rectangle{ 80.0f, 866.0f, 380.0f, 64.0f };
        claimBtn.label = "CLAIM TODAY'S REWARD (+120)";
        claimBtn.cornerRadius = 16.0f;
        claimBtn.fontSize = 16.0f;
        claimBtn.topColor = Color{ 245, 158, 11, 255 };
        claimBtn.bottomColor = Color{ 217, 119, 6, 255 };
        claimBtn.lipColor = Color{ 180, 83, 9, 255 };
        claimBtn.borderColor = Color{ 254, 240, 138, 255 };
    }

    int update(float dt) {
        Vector2 m = Input::Mouse();
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        closeBtn.update(m, mouseDown, dt);
        if (closeBtn.isClicked(m, mouseReleased)) {
            gAudio.playClick();
            return Nav::SCREEN_HOME;
        }

        claimBtn.isDisabled = rewardClaimedToday;
        claimBtn.label = rewardClaimedToday ? "CLAIMED TODAY" : "CLAIM TODAY'S REWARD (+120)";
        claimBtn.update(m, mouseDown, dt);

        if (claimBtn.isClicked(m, mouseReleased)) {
            if (!rewardClaimedToday) {
                rewardClaimedToday = true;
                currentStreakDay = std::min(7, currentStreakDay + 1);
                gGameData.streak_day = currentStreakDay;
                Theme::coins += 120;
                gGameData.saveProfile();
                gAudio.playFanfare();
                Theme::spawnCelebration({ 270, 898 }, 72);
            } else {
                gAudio.playPop();
            }
        }

        // Grand Chest interaction
        Rectangle chestRec{ 280, 540, 175, 175 };
        if (CheckCollisionPointRec(m, chestRec)) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gAudio.playChime();
                Theme::spawnCelebration({ 368, 625 }, 48);
            }
        }

        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. 16-Frame Living Daily Streak 1-to-7 Screen
        if (!UIAssets::drawFrameSequence("streak_living/frame_", 16, t, 12.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 45, 25, 15, 255 }, Color{ 20, 12, 8, 255 });
        }

        // Top HUD (Close button)
        closeBtn.draw();

        // Claim CTA Button
        claimBtn.draw();

        Theme::drawParticles();
    }
};
