#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

class ScreenMaps {
public:
    struct NodeDef {
        float x;
        float y;
        int number;
    };

    struct UnitNodes {
        std::vector<NodeDef> nodes;
        float signX = 0.0f, signY = 0.0f, signW = 0.0f, signH = 0.0f;
        bool hasSign = false;
    };

    int unit = 0; // 0=Ethics, 1=Safety First, 2=Astronomy/Space, 3=Graduation Academy (Scrolls 1-14)
    float scrollY = 0.0f;
    float targetScrollY = 0.0f;
    int selectedLevel = -1;
    int pendingNavScreen = -1; // For switching to Home, Shop, Streak
    std::vector<UnitNodes> units;

    // Graduation Academy Boy march state along scrolls 1 -> 14
    int boyTargetScroll = 4;
    float boyProgress = 3.0f; // 0-based index along the 14 scrolls
    bool boyMarching = false;

    void init() {
        units.resize(4);

        // Unit 0: Ethics Island (10 nodes)
        units[0].nodes = {
            { 270, 760, 1 }, { 215, 712, 2 }, { 195, 655, 3 }, { 248, 608, 4 }, { 315, 568, 5 },
            { 330, 512, 6 }, { 268, 472, 7 }, { 212, 430, 8 }, { 248, 382, 9 }, { 275, 330, 10 }
        };
        units[0].hasSign = true;
        units[0].signX = 160; units[0].signY = 820; units[0].signW = 220; units[0].signH = 110;

        // Unit 1: Safety First / Food Pyramid Island (10 nodes)
        units[1].nodes = {
            { 268, 855, 1 }, { 218, 795, 2 }, { 292, 738, 3 }, { 235, 675, 4 }, { 295, 612, 5 },
            { 230, 548, 6 }, { 285, 482, 7 }, { 235, 415, 8 }, { 280, 342, 9 }, { 268, 260, 10 }
        };

        // Unit 2: Space / Astronomy Observatory Island (10 nodes)
        units[2].nodes = {
            { 270, 825, 1 }, { 340, 760, 2 }, { 225, 705, 3 }, { 188, 635, 4 }, { 285, 575, 5 },
            { 355, 505, 6 }, { 255, 445, 7 }, { 195, 385, 8 }, { 255, 325, 9 }, { 235, 265, 10 }
        };

        // Unit 3: Graduation Academy Map (14 Numbered Scrolls + Telegrams reaching the Grand Blue Castle)
        units[3].nodes = {
            { 165, 845, 1 },
            { 268, 825, 2 },
            { 372, 792, 3 },
            { 312, 738, 4 },
            { 195, 702, 5 },
            { 152, 642, 6 },
            { 248, 598, 7 },
            { 358, 560, 8 },
            { 382, 495, 9 },
            { 282, 452, 10 }, // Entering the Giant Open Book Bridge
            { 225, 392, 11 }, // Middle of the Giant Open Book Bridge
            { 278, 338, 12 }, // Upper Book Bridge
            { 335, 282, 13 }, // Castle Courtyard
            { 285, 222, 14 }  // Grand Blue Castle Gate!
        };
    }

    void setUnit(int u) {
        unit = std::clamp(u, 0, 3);
        selectedLevel = -1;
    }

    int update(float dt) {
        selectedLevel = -1;
        pendingNavScreen = -1;

        // Animate Boy walking along the 14 scrolls toward the Blue Castle
        if (boyMarching) {
            boyProgress += dt * 2.2f;
            if (boyProgress >= 13.0f) {
                boyProgress = 13.0f;
                boyMarching = false;
                gAudio.playFanfare();
                Theme::spawnCelebration({ 285, 200 }, 64);
            }
        } else {
            float target = (float)std::clamp(boyTargetScroll - 1, 0, 13);
            if (std::abs(boyProgress - target) > 0.02f) {
                float dir = (target > boyProgress) ? 1.0f : -1.0f;
                boyProgress += dir * dt * 3.0f;
                if ((dir > 0 && boyProgress > target) || (dir < 0 && boyProgress < target)) {
                    boyProgress = target;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();

            // Top Unit Selector Tabs (0=Ethics, 1=Safety, 2=Space, 3=Graduation Academy)
            for (int u = 0; u < 4; ++u) {
                Rectangle tabRec{ 12.0f + u * 130.0f, 72.0f, 124.0f, 38.0f };
                if (CheckCollisionPointRec(m, tabRec)) {
                    setUnit(u);
                    gAudio.playClick();
                    return -1;
                }
            }

            // Bottom Navigation Buttons: Home, Shop, Streak, March Boy (on Academy Map)
            if (CheckCollisionPointRec(m, Rectangle{ 14, 892, 118, 56 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_HOME;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 142, 892, 118, 56 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_SHOP;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 270, 892, 118, 56 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_STREAK;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 398, 892, 128, 56 })) {
                if (unit == 3) {
                    boyProgress = 0.0f;
                    boyTargetScroll = 14;
                    boyMarching = true;
                    gAudio.playFanfare();
                } else {
                    setUnit((unit + 1) % 4);
                    gAudio.playClick();
                }
                return -1;
            }

            // Node / Scroll clicks
            const auto& nodes = units[unit].nodes;
            for (size_t i = 0; i < nodes.size(); ++i) {
                float hitRadius = (unit == 3) ? 34.0f : 28.0f;
                if (CheckCollisionPointCircle(m, { nodes[i].x, nodes[i].y }, hitRadius)) {
                    gAudio.playPop();
                    if (unit == 3) {
                        boyTargetScroll = (int)i + 1;
                        boyMarching = false;
                    }
                    int totalLvls = std::max(1, gGameData.levelCount());
                    int startIdx = (unit < 3) ? gGameData.unitStart(unit) : 0;
                    selectedLevel = (startIdx + (int)i) % totalLvls;
                    return selectedLevel;
                }
            }
        }
        return -1;
    }

    void drawGraduationAcademyMap(int w, int h) {
        float t = Theme::timeNow;

        // 1. Clean Academy Map Backdrop
        if (!UIAssets::drawStretch("image2_graduation_academy_map_assets/map_background_clean.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 186, 230, 253, 255 });
        }

        // 2. Giant Open Book Bridge, Clock Tower, and Grand Blue Castle
        UIAssets::drawCentered("image2_graduation_academy_map_assets/bridge_giant_open_book.png",
                               { 255, 405 }, 255, 165);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/building_clock_tower.png",
                               { 102, 235 }, 145, 225);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/building_big_blue_castle.png",
                               { 312, 165 }, 290, 245);

        // 3. Waving Castle Flags atop the Grand Blue Castle
        bool flagPose2 = ((int)(t * 4.5f) % 2) == 1;
        float flagTilt = std::sin(t * 5.0f) * 6.0f;
        const char* flagKey = flagPose2
            ? "image2_graduation_academy_map_assets/prop_castle_flag_pose2.png"
            : "image2_graduation_academy_map_assets/prop_castle_flag_pose1.png";
        UIAssets::drawCentered(flagKey, { 248, 68 }, 54, 54, flagTilt);
        UIAssets::drawCentered(flagKey, { 372, 68 }, 54, 54, -flagTilt);

        // 4. Spinning Globe, Chalkboard, ABC Blocks & Flying Magic Books
        float globeTilt = std::sin(t * 2.8f) * 12.0f;
        UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_globe_full.png",
                               { 442, 650 }, 118, 135, globeTilt);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_chalkboard.png",
                               { 95, 515 }, 135, 120);
        UIAssets::drawCentered("image2_graduation_academy_map_assets/island_abc_blocks.png",
                               { 88, 795 }, 135, 115);

        bool bookPose2 = ((int)(t * 5.2f) % 2) == 1;
        const char* bookKey = bookPose2
            ? "image2_graduation_academy_map_assets/prop_flying_book_pose2.png"
            : "image2_graduation_academy_map_assets/prop_flying_book_pose1.png";
        UIAssets::drawCentered(bookKey, { 125, 355 + std::sin(t * 4.0f) * 9.0f }, 78, 68, std::sin(t * 3.5f) * 8.0f);
        UIAssets::drawCentered(bookKey, { 435, 365 + std::cos(t * 3.8f) * 9.0f }, 74, 64, std::cos(t * 3.2f) * -8.0f);

        // 5. Golden Path Connecting all 14 Scrolls & Telegrams up to the Grand Blue Castle
        const auto& nodes = units[3].nodes;
        for (size_t i = 0; i + 1 < nodes.size(); ++i) {
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       8.0f, Color{ 251, 191, 36, 175 });
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       3.5f, Color{ 254, 249, 195, 235 });
        }

        // Draw all 14 Numbered Scrolls (scroll_step_01.png .. scroll_step_14.png)
        for (size_t i = 0; i < nodes.size(); ++i) {
            char scrollKey[128];
            std::snprintf(scrollKey, sizeof(scrollKey),
                          "image2_graduation_academy_map_assets/numbered_scrolls_1_to_14/scroll_step_%02d.png",
                          (int)i + 1);
            float pulse = ((int)i == (int)std::round(boyProgress)) ? (1.0f + 0.08f * std::sin(t * 5.0f)) : 1.0f;
            if (!UIAssets::drawCentered(scrollKey, { nodes[i].x, nodes[i].y }, 86 * pulse, 64 * pulse)) {
                UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_scroll_and_telegram_step.png",
                                       { nodes[i].x, nodes[i].y }, 86 * pulse, 64 * pulse);
                Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y - 4.0f },
                              18, Color{ 120, 53, 15, 255 }, WHITE, 2.0f, BLANK, true, true);
            }
        }

        // 6. Animated Owl Flapping Above the Academy
        bool owlFlap = ((int)(t * 4.8f) % 2) == 1;
        const char* owlKey = owlFlap
            ? "image2_graduation_academy_map_assets/char_owl_pose2_flapping.png"
            : "image2_graduation_academy_map_assets/char_owl_pose1_idle.png";
        UIAssets::drawCentered(owlKey, { 418, 225 + std::sin(t * 3.6f) * 10.0f }, 96, 96);

        // 7. Animated Boy Student Moving Across Scrolls (1 -> 14) to the Blue Castle
        int idxA = std::clamp((int)std::floor(boyProgress), 0, 13);
        int idxB = std::clamp(idxA + 1, 0, 13);
        float frac = boyProgress - (float)idxA;
        Vector2 boyPos{
            nodes[idxA].x + (nodes[idxB].x - nodes[idxA].x) * frac,
            nodes[idxA].y + (nodes[idxB].y - nodes[idxA].y) * frac - 34.0f
        };
        float hopY = std::abs(std::sin(t * 5.5f)) * -12.0f;
        bool boyJump = ((int)(t * 4.0f) % 2) == 1 || boyMarching;
        const char* boyKey = boyJump
            ? "image2_graduation_academy_map_assets/char_boy_pose2_jumping.png"
            : "image2_graduation_academy_map_assets/char_boy_pose1_cheering.png";
        UIAssets::drawCentered(boyKey, { boyPos.x, boyPos.y + hopY }, 105, 118);

        // 8. Graduation Academy Banner
        UIAssets::drawCentered("image2_graduation_academy_map_assets/prop_graduation_academy_banner.png",
                               { 270, 132 }, 320, 66);
    }

    void drawSafetyFoodIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawStretch("assets/scenery/clean_safety.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 30, 136, 229, 255 }, Color{ 2, 119, 189, 255 });
        }

        // Animated Chef & Dog Inspector from food_pyramid_island_ui_kit
        float chefBob = std::sin(t * 3.2f) * 5.0f;
        UIAssets::drawCentered("food_pyramid_island_ui_kit/02_characters_animated/chef_mascot_waving.png",
                               { 355, 455 + chefBob }, 125, 135);
        UIAssets::drawCentered("food_pyramid_island_ui_kit/02_characters_animated/dog_inspector_full.png",
                               { 420, 645 + std::cos(t * 3.5f) * 4.0f }, 105, 105);

        const auto& nodes = units[1].nodes;
        for (size_t i = 0; i < nodes.size(); ++i) {
            char key[128];
            std::snprintf(key, sizeof(key),
                          "food_pyramid_island_ui_kit/04_level_buttons_1_to_10/level_%02d_button.png",
                          (int)i + 1);
            if (!UIAssets::drawCentered(key, { nodes[i].x, nodes[i].y }, 68, 68)) {
                DrawCircleV({ nodes[i].x, nodes[i].y }, 26.0f, Color{ 245, 158, 11, 255 });
                Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y }, 20, WHITE, BLACK, 2.0f, BLACK, true, true);
            }
        }
    }

    void drawCosmicSpaceIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawStretch("assets/scenery/clean_space.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 11, 15, 41, 255 }, Color{ 46, 16, 101, 255 });
        }

        // Animated Astronaut, Rocket & Satellite from cosmic_space_observatory_ui_kit
        UIAssets::drawCentered("cosmic_space_observatory_ui_kit/03_characters_and_vehicles/astronaut_with_star_flag.png",
                               { 295, 175 + std::sin(t * 2.8f) * 7.0f }, 120, 120);
        UIAssets::drawCentered("cosmic_space_observatory_ui_kit/03_characters_and_vehicles/space_satellite_solar_wings.png",
                               { 435, 215 + std::cos(t * 2.4f) * 9.0f }, 115, 95, std::sin(t * 1.8f) * 6.0f);

        const auto& nodes = units[2].nodes;
        for (size_t i = 0; i < nodes.size(); ++i) {
            char key[128];
            std::snprintf(key, sizeof(key),
                          "cosmic_space_observatory_ui_kit/04_level_nodes_01_to_10/node_%02d_cyan.png",
                          (int)i + 1);
            if (!UIAssets::drawCentered(key, { nodes[i].x, nodes[i].y }, 68, 68)) {
                DrawCircleV({ nodes[i].x, nodes[i].y }, 26.0f, Color{ 6, 182, 212, 255 });
                Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y }, 20, WHITE, BLACK, 2.0f, BLACK, true, true);
            }
        }
    }

    void drawEthicsIsland(int w, int h) {
        if (!UIAssets::drawStretch("assets/scenery/clean_ethics.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 174, 226, 255, 255 }, Color{ 243, 232, 255, 255 });
        }
        const auto& nodes = units[0].nodes;
        for (size_t i = 0; i < nodes.size(); ++i) {
            Color c = (i == 0) ? Color{ 14, 165, 233, 255 } : Color{ 148, 163, 184, 255 };
            DrawCircleV({ nodes[i].x, nodes[i].y + 4.0f }, 27.0f, Color{ 15, 23, 42, 140 });
            DrawCircleV({ nodes[i].x, nodes[i].y }, 26.0f, c);
            DrawCircleLines((int)nodes[i].x, (int)nodes[i].y, 26.0f, WHITE);
            Theme::text3D(std::to_string(i + 1), { nodes[i].x, nodes[i].y }, 20, WHITE, BLACK, 2.0f, BLACK, true, true);
        }
    }

    void draw(int w, int h) {
        if (unit == 0) drawEthicsIsland(w, h);
        else if (unit == 1) drawSafetyFoodIsland(w, h);
        else if (unit == 2) drawCosmicSpaceIsland(w, h);
        else drawGraduationAcademyMap(w, h);

        // Top HUD Header Bar
        UIAssets::drawFit("cosmic_space_observatory_ui_kit/01_hud_bars_and_buttons/hud_top_star_progress_bar.png",
                          Rectangle{ 14, 10, 260, 52 });
        UIAssets::drawFit("cosmic_space_observatory_ui_kit/01_hud_bars_and_buttons/hud_top_coin_counter_pill.png",
                          Rectangle{ 310, 12, 145, 48 });
        Theme::text3D(std::to_string(Theme::coins), { 398, 36 }, 20, Color{ 253, 224, 71, 255 }, BLACK, 2.0f, BLACK, true, true);

        // 4 Unit Tabs (Ethics, Safety, Space, Academy 1-14)
        const char* tabLabels[4] = { "1.ETHICS", "2.SAFETY", "3.SPACE", "4.ACADEMY" };
        for (int u = 0; u < 4; ++u) {
            Rectangle tabRec{ 12.0f + u * 130.0f, 72.0f, 124.0f, 38.0f };
            Color topC = (unit == u) ? Color{ 251, 191, 36, 255 } : Color{ 30, 41, 59, 215 };
            Color botC = (unit == u) ? Color{ 217, 119, 6, 255 } : Color{ 15, 23, 42, 215 };
            UI::GradientRound(tabRec, 14.0f, topC, botC);
            UI::Rim(tabRec, 14.0f, WHITE, WHITE, Color{ 100, 116, 139, 255 });
            Theme::text3D(tabLabels[u], { tabRec.x + 62.0f, tabRec.y + 19.0f },
                          15, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        // Bottom Navigation Bar (Home, Shop, Streak 1-7, March/Next)
        const char* botLabels[4] = { "HOME", "SHOP", "STREAK", (unit == 3) ? "GO 1->14" : "NEXT MAP" };
        Color botCols[4] = {
            Color{ 16, 185, 129, 255 },
            Color{ 124, 58, 237, 255 },
            Color{ 234, 88, 12, 255 },
            Color{ 37, 99, 235, 255 }
        };
        for (int i = 0; i < 4; ++i) {
            Rectangle r{ 14.0f + i * 128.0f, 892.0f, 120.0f, 54.0f };
            UI::DropShadow(r, 16.0f, 4.0f, 0.35f, 2.0f);
            UI::GradientRound(r, 16.0f, botCols[i], Color{ 15, 23, 42, 255 });
            UI::Rim(r, 16.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::text3D(botLabels[i], { r.x + 60.0f, r.y + 27.0f }, 16, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        Theme::drawParticles();
    }
};
