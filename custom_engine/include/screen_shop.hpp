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

class ScreenShop {
public:
    int ownedBoosters[4] = { 3, 2, 1, 1 };
    int prices[4] = { 100, 150, 200, 250 };
    std::string statusMsg;
    float statusTimer = 0.0f;

    UI::StatefulButton closeBtn;
    UI::StatefulButton buyButtons[4];

    void init() {
        statusMsg.clear();
        statusTimer = 0.0f;

        closeBtn.bounds = Rectangle{ 456.0f, 12.0f, 72.0f, 42.0f };
        closeBtn.label = "CLOSE";
        closeBtn.cornerRadius = 12.0f;
        closeBtn.fontSize = 13.0f;
        closeBtn.topColor = Color{ 239, 68, 68, 245 };
        closeBtn.bottomColor = Color{ 185, 28, 28, 245 };
        closeBtn.lipColor = Color{ 153, 27, 27, 245 };
        closeBtn.borderColor = Color{ 254, 202, 202, 255 };

        Rectangle buyRects[4] = {
            { 292.0f, 258.0f, 156.0f, 52.0f },
            { 292.0f, 444.0f, 156.0f, 52.0f },
            { 292.0f, 630.0f, 156.0f, 52.0f },
            { 292.0f, 816.0f, 156.0f, 52.0f }
        };

        for (int i = 0; i < 4; ++i) {
            buyButtons[i].bounds = buyRects[i];
            buyButtons[i].label = std::to_string(prices[i]) + " COINS";
            buyButtons[i].cornerRadius = 14.0f;
            buyButtons[i].fontSize = 14.0f;
            buyButtons[i].topColor = Color{ 16, 185, 129, 245 };
            buyButtons[i].bottomColor = Color{ 5, 150, 105, 245 };
            buyButtons[i].lipColor = Color{ 6, 95, 70, 245 };
            buyButtons[i].borderColor = Color{ 167, 243, 208, 255 };
        }
    }

    int update(float dt) {
        if (statusTimer > 0.0f) {
            statusTimer = std::max(0.0f, statusTimer - dt);
        }

        Vector2 m = Input::Mouse();
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        closeBtn.update(m, mouseDown, dt);
        if (closeBtn.isClicked(m, mouseReleased)) {
            gAudio.playClick();
            return Nav::SCREEN_HOME;
        }

        for (int i = 0; i < 4; ++i) {
            buyButtons[i].isDisabled = (Theme::coins < prices[i]);
            buyButtons[i].update(m, mouseDown, dt);

            if (buyButtons[i].isClicked(m, mouseReleased)) {
                if (Theme::coins >= prices[i]) {
                    Theme::coins -= prices[i];
                    ownedBoosters[i]++;
                    gGameData.saveProfile();
                    gAudio.playFanfare();
                    Theme::spawnCelebration({ buyButtons[i].bounds.x + 75.0f, buyButtons[i].bounds.y + 26.0f }, 36);
                    statusMsg = "تم شراء المعزز بنجاح!";
                    statusTimer = 2.2f;
                } else {
                    gAudio.playPop();
                    statusMsg = "رصيد العملات غير كافٍ!";
                    statusTimer = 2.0f;
                }
            }
        }

        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. 16-Frame Living Alchemist Booster Shop
        if (!UIAssets::drawFrameSequence("shop_living/frame_", 16, t, 12.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 28, 18, 56, 255 }, Color{ 14, 9, 32, 255 });
        }

        // Top HUD (Coins pill + Close button)
        Rectangle coinPill{ 16, 12, 130, 42 };
        UI::GlassPanel(coinPill, 14.0f, Color{ 30, 41, 59, 230 }, Color{ 250, 204, 21, 255 });
        UI::CoinIcon({ 38, 33 }, 13.0f);
        Theme::text3D(std::to_string(Theme::coins), { 88, 33 }, 17, WHITE, BLACK, 2.0f, BLACK, true, true);

        closeBtn.draw();

        // 2. Owned Booster Count Badges
        float badgeY[4] = { 186.0f, 372.0f, 558.0f, 744.0f };
        for (int i = 0; i < 4; ++i) {
            Vector2 badgeCenter{ 468.0f, badgeY[i] };
            DrawCircleV(badgeCenter, 18.0f, Color{ 16, 185, 129, 255 });
            DrawCircleLines((int)badgeCenter.x, (int)badgeCenter.y, 18.0f, WHITE);
            Theme::text3D("x" + std::to_string(ownedBoosters[i]), badgeCenter, 15, WHITE, BLACK, 2.0f, BLACK, true, true);

            // Draw Stateful Buy Button
            buyButtons[i].draw();
        }

        if (statusTimer > 0.0f && !statusMsg.empty()) {
            Rectangle toast{ 90, 895, 360, 50 };
            UI::GradientRound(toast, 16.0f, Color{ 15, 23, 42, 235 }, Color{ 30, 58, 138, 235 });
            UI::Rim(toast, 16.0f, Color{ 250, 204, 21, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::textArabic(statusMsg, { 270, 920 }, 22, Color{ 254, 240, 138, 255 }, true, true, BLACK, 2.0f);
        }

        Theme::drawParticles();
    }
};
