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

    void init() {
        statusMsg.clear();
        statusTimer = 0.0f;
    }

    int update(float dt) {
        if (statusTimer > 0.0f) {
            statusTimer = std::max(0.0f, statusTimer - dt);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Close button (top right of Booster Shop)
            if (CheckCollisionPointRec(m, Rectangle{ 452, 12, 72, 72 })) {
                gAudio.playClick();
                return Nav::SCREEN_HOME;
            }

            // 4 Stacked Horizontal Booster Cards Buy Buttons (matching ref9_booster_shop.jpg!)
            Rectangle buyRects[4] = {
                { 295, 255, 155, 62 },
                { 295, 442, 155, 62 },
                { 295, 628, 155, 62 },
                { 295, 815, 155, 62 }
            };

            for (int i = 0; i < 4; ++i) {
                if (CheckCollisionPointRec(m, buyRects[i])) {
                    if (Theme::coins >= prices[i]) {
                        Theme::coins -= prices[i];
                        ownedBoosters[i]++;
                        gGameData.saveProfile();
                        gAudio.playFanfare();
                        Theme::spawnCelebration({ buyRects[i].x + 75.0f, buyRects[i].y + 30.0f }, 36);
                        statusMsg = "تم شراء المعزز بنجاح!";
                        statusTimer = 2.2f;
                    } else {
                        gAudio.playPop();
                        statusMsg = "رصيد العملات غير كافٍ!";
                        statusTimer = 2.0f;
                    }
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. 16-Frame Living Alchemist Booster Shop (Flickering candles, glowing potions & static 4-row Booster Cards!)
        if (!UIAssets::drawFrameSequence("shop_living/frame_", 16, t, 12.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 28, 18, 56, 255 }, Color{ 14, 9, 32, 255 });
        }

        // 2. Owned Booster Count Badges on the top-right corner of each of the 4 stacked cards
        float badgeY[4] = { 186.0f, 372.0f, 558.0f, 744.0f };
        for (int i = 0; i < 4; ++i) {
            Vector2 badgeCenter{ 468.0f, badgeY[i] };
            DrawCircleV(badgeCenter, 18.0f, Color{ 16, 185, 129, 255 });
            DrawCircleLines((int)badgeCenter.x, (int)badgeCenter.y, 18.0f, WHITE);
            Theme::text3D("x" + std::to_string(ownedBoosters[i]), badgeCenter, 15, WHITE, BLACK, 2.0f, BLACK, true, true);
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
