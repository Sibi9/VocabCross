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
            // Close button (top right)
            if (CheckCollisionPointRec(m, Rectangle{ 462, 18, 62, 62 })) {
                gAudio.playClick();
                return Nav::SCREEN_HOME;
            }

            // 4 Buy Buttons inside 2x2 Booster Grid
            Rectangle buyRects[4] = {
                { 58, 462, 180, 56 },
                { 302, 462, 180, 56 },
                { 58, 792, 180, 56 },
                { 302, 792, 180, 56 }
            };

            for (int i = 0; i < 4; ++i) {
                if (CheckCollisionPointRec(m, buyRects[i])) {
                    if (Theme::coins >= prices[i]) {
                        Theme::coins -= prices[i];
                        ownedBoosters[i]++;
                        gAudio.playFanfare();
                        Theme::spawnCelebration({ buyRects[i].x + 90.0f, buyRects[i].y + 28.0f }, 36);
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

        // 1. Clean Alchemist Study Background
        if (!UIAssets::drawStretch("magical_alchemist_booster_shop_ui_kit/01_backgrounds/bg_alchemist_study_clean.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 28, 18, 56, 255 }, Color{ 14, 9, 32, 255 });
        }

        // 2. LIVING BACKGROUND FX (Flickering Candles, Glowing Potions, Twinkling Stars & Floating Magic Dust)
        // Note: All Shop UI Cards & Buttons remain 100% static as required!
        Vector2 candleSpots[4] = { { 72, 195 }, { 478, 198 }, { 48, 520 }, { 495, 515 } };
        for (int i = 0; i < 4; ++i) {
            float flicker = 0.75f + 0.25f * std::sin(t * (7.5f + i * 1.9f) + i)
                                  + 0.12f * std::cos(t * (13.0f + i * 2.3f));
            float rad = 36.0f * flicker;
            DrawCircleGradient((int)candleSpots[i].x, (int)candleSpots[i].y, rad,
                               Color{ 255, 200, 80, (unsigned char)(115 * flicker) },
                               Color{ 255, 140, 20, 0 });
        }

        // Rising golden dust particles in the background
        for (int i = 0; i < 28; ++i) {
            float px = std::fmod(i * 73.0f + std::sin(t * 1.2f + i) * 24.0f, (float)w);
            float py = h - std::fmod(t * (22.0f + (i % 5) * 6.0f) + i * 47.0f, (float)h);
            float alpha = 0.45f + 0.45f * std::sin(t * 3.5f + i * 1.7f);
            DrawCircleV({ px, py }, 2.2f + (i % 3), Color{ 253, 224, 71, (unsigned char)(180 * alpha) });
        }

        // 3. Static Shop Header, Coin Bar, and Close Button
        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/hud_coin_bar_blank.png",
                          Rectangle{ 16, 18, 175, 56 });
        Theme::text3D(std::to_string(Theme::coins), { 112, 46 }, 24, Color{ 253, 224, 71, 255 }, BLACK, 2.5f, BLACK, true, true);

        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/banner_booster_shop.png",
                          Rectangle{ 95, 78, 350, 92 });

        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/02_blank_ui_templates/btn_close_red_x.png",
                          Rectangle{ 462, 18, 62, 62 });

        // 4. 2x2 Static Booster Cards + Buy Buttons
        const char* cardKeys[4] = {
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_1_magic_lightbulb.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_2_detective_magnifier.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_3_rocket_eraser.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/card_4_master_dictionary.png"
        };
        const char* buyKeys[4] = {
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_100.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_150.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_200.png",
            "magical_alchemist_booster_shop_ui_kit/04_sliced_exact_components/btn_buy_250.png"
        };
        Rectangle cardRects[4] = {
            { 32, 188, 232, 268 },
            { 276, 188, 232, 268 },
            { 32, 518, 232, 268 },
            { 276, 518, 232, 268 }
        };
        Rectangle buyRects[4] = {
            { 58, 462, 180, 56 },
            { 302, 462, 180, 56 },
            { 58, 792, 180, 56 },
            { 302, 792, 180, 56 }
        };

        for (int i = 0; i < 4; ++i) {
            UIAssets::drawFit(cardKeys[i], cardRects[i]);
            UIAssets::drawFit(buyKeys[i], buyRects[i]);

            // Owned count badge in top-right corner of each card
            Vector2 badgeCenter{ cardRects[i].x + cardRects[i].width - 24.0f, cardRects[i].y + 22.0f };
            DrawCircleV(badgeCenter, 18.0f, Color{ 16, 185, 129, 255 });
            DrawCircleLines((int)badgeCenter.x, (int)badgeCenter.y, 18.0f, WHITE);
            Theme::text3D("x" + std::to_string(ownedBoosters[i]), badgeCenter, 16, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        if (statusTimer > 0.0f && !statusMsg.empty()) {
            Rectangle toast{ 90, 875, 360, 54 };
            UI::GradientRound(toast, 16.0f, Color{ 15, 23, 42, 235 }, Color{ 30, 58, 138, 235 });
            UI::Rim(toast, 16.0f, Color{ 250, 204, 21, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::textArabic(statusMsg, { 270, 902 }, 22, Color{ 254, 240, 138, 255 }, true, true, BLACK, 2.0f);
        }

        Theme::drawParticles();
    }
};
