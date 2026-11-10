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
            // Back / Close button
            if (CheckCollisionPointRec(m, Rectangle{ 16, 18, 56, 56 }) ||
                CheckCollisionPointRec(m, Rectangle{ 468, 18, 56, 56 })) {
                gAudio.playClick();
                return Nav::SCREEN_HOME;
            }
            // Claim Today's Reward CTA Button
            if (CheckCollisionPointRec(m, Rectangle{ 75, 848, 390, 82 })) {
                if (!rewardClaimedToday) {
                    rewardClaimedToday = true;
                    currentStreakDay = std::min(7, currentStreakDay + 1);
                    Theme::coins += 120;
                    gAudio.playFanfare();
                    Theme::spawnCelebration({ 270, 880 }, 72);
                } else {
                    gAudio.playPop();
                }
            }
            // Clicking Grand Chest triggers vibration burst
            if (CheckCollisionPointRec(m, Rectangle{ 260, 235, 220, 175 })) {
                gAudio.playChime();
                Theme::spawnCelebration({ 370, 315 }, 48);
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;

        // 1. Clean Forest & Roadmap Base (without baked owl or chest so they animate cleanly)
        if (!UIAssets::drawStretch("daily_streak_roadmap_ui_kit/01_screens_and_boards/daily_streak_clean_base_no_owl_no_chest.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            UIAssets::drawStretch("daily_streak_roadmap_ui_kit/01_screens_and_boards/roadmap_board_clean_1_to_7.png",
                                  Rectangle{ 0, 0, (float)w, (float)h });
        }

        // 2. Top Header Banner, 12-Days Streak Badge & 14-Days Exam Countdown Banner
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/header_daily_streak_title.png",
                          Rectangle{ 75, 16, 390, 85 });
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/badge_streak_12_days.png",
                          Rectangle{ 95, 102, 350, 72 });
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/banner_14_days_exam.png",
                          Rectangle{ 58, 178, 424, 58 });

        // Close Button (top right)
        UIAssets::drawFit("magical_alchemist_booster_shop_ui_kit/02_blank_ui_templates/btn_close_red_x.png",
                          Rectangle{ 470, 20, 54, 54 });

        // 3. Corrected Sequential Day 1 -> Day 7 Stumps along the Winding Path
        struct DayNode {
            const char* key;
            Vector2 pos;
            Vector2 size;
            const char* label;
        };
        DayNode days[7] = {
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_1.png", { 118, 725 }, { 108, 95 }, "DAY 1" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_2.png", { 265, 718 }, { 108, 95 }, "DAY 2" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_3.png", { 412, 682 }, { 108, 95 }, "DAY 3" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_4.png", { 378, 565 }, { 108, 95 }, "DAY 4" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_5.png", { 228, 548 }, { 108, 95 }, "DAY 5" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_6.png", { 225, 435 }, { 108, 95 }, "DAY 6" },
            { "daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/stump_day_7_pedestal.png", { 372, 385 }, { 165, 110 }, "DAY 7" }
        };

        for (int i = 0; i < 7; ++i) {
            float pulse = (i + 1 == currentStreakDay) ? (1.0f + 0.06f * std::sin(t * 4.5f)) : 1.0f;
            UIAssets::drawCentered(days[i].key, days[i].pos, days[i].size.x * pulse, days[i].size.y * pulse);

            Color lblCol = (i + 1 <= currentStreakDay) ? Color{ 253, 224, 71, 255 } : WHITE;
            Theme::text3D(days[i].label, { days[i].pos.x, days[i].pos.y + 38.0f },
                          16, lblCol, Color{ 66, 32, 6, 255 }, 2.5f, BLACK, true, true);
        }

        // 4. Vibrating Grand BAC Mega Bonus Chest on Day 7 Pedestal
        float shakePhase = std::sin(t * 28.0f) * std::max(0.0f, std::sin(t * 2.6f));
        float chestRot = shakePhase * 6.5f;
        float chestScale = 1.0f + std::abs(shakePhase) * 0.05f;
        float chestJump = std::abs(shakePhase) * -7.0f;

        DrawCircleGradient(372, 318, 95.0f, Color{ 253, 224, 71, 95 }, Color{ 253, 224, 71, 0 });
        UIAssets::drawCentered("daily_streak_roadmap_ui_kit/02_characters_and_chest/prop_bac_mega_bonus_chest_hd.png",
                               { 372, 318 + chestJump }, 185 * chestScale, 155 * chestScale, chestRot);

        // 5. Animated Professor Owl Mascot (Pose A <-> Pose B + Bobbing & Tilt)
        float owlBob = std::sin(t * 3.2f) * 7.0f;
        float owlTilt = std::sin(t * 2.1f) * 3.5f;
        bool poseB = ((int)(t * 2.4f) % 2) == 1;
        const char* owlKey = poseB
            ? "daily_streak_roadmap_ui_kit/02_characters_and_chest/char_professor_owl_pose_b.png"
            : "daily_streak_roadmap_ui_kit/02_characters_and_chest/char_professor_owl_pose_a.png";
        UIAssets::drawCentered(owlKey, { 112, 385 + owlBob }, 195, 225, owlTilt);

        // 6. Claim Today's Reward CTA Button
        Rectangle claimRec{ 75, 848, 390, 82 };
        UIAssets::drawFit("daily_streak_roadmap_ui_kit/03_ui_and_roadmap_items/btn_claim_todays_reward.png", claimRec);

        Theme::drawParticles();
    }
};
