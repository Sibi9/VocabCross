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

    void init() {
        currentStreakDay = 4;
        rewardClaimedToday = false;
    }

    int update(float dt) {
        (void)dt;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Top-right Close button
            if (CheckCollisionPointRec(m, Rectangle{ 468, 16, 56, 56 })) {
                gAudio.playClick();
                return Nav::SCREEN_HOME;
            }
            // Claim Today's Reward CTA Button
            if (CheckCollisionPointRec(m, Rectangle{ 85, 868, 370, 82 })) {
                if (!rewardClaimedToday) {
                    rewardClaimedToday = true;
                    currentStreakDay = std::min(7, currentStreakDay + 1);
                    gGameData.streak_day = currentStreakDay;
                    Theme::coins += 120;
                    gGameData.saveProfile();
                    gAudio.playFanfare();
                    Theme::spawnCelebration({ 270, 900 }, 72);
                } else {
                    gAudio.playPop();
                }
            }
            // Clicking Grand Chest triggers celebration burst
            if (CheckCollisionPointRec(m, Rectangle{ 280, 540, 175, 175 })) {
                gAudio.playChime();
                Theme::spawnCelebration({ 368, 625 }, 48);
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. 16-Frame Living Daily Streak 1-to-7 Screen (Corrected Day 1..7 Roadmap, Animated Professor Owl & Shaking Mega Bonus Chest!)
        if (!UIAssets::drawFrameSequence("streak_living/frame_", 16, t, 12.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 45, 25, 15, 255 }, Color{ 20, 12, 8, 255 });
        }

        // 2. Top-Right Close Button
        UIAssets::drawFit("hud/btn_close_red_x.png", Rectangle{ 470, 18, 54, 54 });

        Theme::drawParticles();
    }
};
