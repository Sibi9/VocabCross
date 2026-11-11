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
    int activeNodeIdx = 0;
    int hoveredNodeIdx = -1;
    int pendingNavScreen = -1;
    std::vector<UnitNodes> units;

    int boyTargetScroll = 4;
    float boyProgress = 3.0f;
    bool boyMarching = false;

    void init() {
        units.resize(4);

        // Verified pixel-exact centers of the 10 3D nodes on Unit 0: Ethics Island (540x960)
        units[0].nodes = {
            { 268, 795, 1 }, { 315, 765, 2 }, { 338, 705, 3 }, { 312, 646, 4 }, { 262, 606, 5 },
            { 235, 551, 6 }, { 276, 464, 7 }, { 246, 405, 8 }, { 228, 376, 9 }, { 264, 324, 10 }
        };
        units[0].hasSign = true;
        units[0].signX = 160; units[0].signY = 820; units[0].signW = 220; units[0].signH = 110;

        // Verified pixel-exact centers of the 10 citrus nodes on Unit 1: Safety First Food Island (540x960)
        units[1].nodes = {
            { 276, 892, 1 }, { 378, 830, 2 }, { 404, 774, 3 }, { 355, 725, 4 }, { 268, 566, 5 },
            { 225, 526, 6 }, { 256, 488, 7 }, { 300, 444, 8 }, { 296, 368, 9 }, { 286, 252, 10 }
        };

        // Verified pixel-exact centers of the 10 neon nodes on Unit 2: Space Observatory Island (540x960)
        units[2].nodes = {
            { 258, 822, 1 }, { 138, 728, 2 }, { 248, 654, 3 }, { 372, 588, 4 }, { 402, 495, 5 },
            { 312, 472, 6 }, { 222, 398, 7 }, { 268, 325, 8 }, { 338, 265, 9 }, { 295, 205, 10 }
        };

        // Unit 3: Graduation Academy Map (14 Numbered Scrolls reaching the Grand Blue Castle)
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
            { 282, 452, 10 },
            { 225, 392, 11 },
            { 278, 338, 12 },
            { 335, 282, 13 },
            { 285, 222, 14 }
        };
    }

    void setUnit(int u) {
        unit = std::clamp(u, 0, 3);
        selectedLevel = -1;
        activeNodeIdx = 0;
        hoveredNodeIdx = -1;
    }

    int update(float dt) {
        selectedLevel = -1;
        pendingNavScreen = -1;
        hoveredNodeIdx = -1;

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

        Vector2 m = Input::Mouse();
        const auto& nodes = units[unit].nodes;
        float hitRadius = (unit == 3) ? 38.0f : 32.0f;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (CheckCollisionPointCircle(m, { nodes[i].x, nodes[i].y }, hitRadius)) {
                hoveredNodeIdx = (int)i;
                break;
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Top Unit Selector Tabs (y=6..50)
            for (int u = 0; u < 4; ++u) {
                Rectangle tabRec{ 10.0f + u * 131.0f, 8.0f, 125.0f, 40.0f };
                if (CheckCollisionPointRec(m, tabRec)) {
                    setUnit(u);
                    gAudio.playClick();
                    return -1;
                }
            }

            // Bottom Corner Navigation Pills (y=908..956)
            if (CheckCollisionPointRec(m, Rectangle{ 8, 908, 96, 46 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_HOME;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 108, 908, 90, 46 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_SHOP;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 342, 908, 92, 46 })) {
                gAudio.playClick();
                pendingNavScreen = Nav::SCREEN_STREAK;
                return -1;
            }
            if (CheckCollisionPointRec(m, Rectangle{ 438, 908, 94, 46 })) {
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

            // Node / Scroll click
            if (hoveredNodeIdx >= 0 && hoveredNodeIdx < (int)nodes.size()) {
                gAudio.playPop();
                activeNodeIdx = hoveredNodeIdx;
                if (unit == 3) {
                    boyTargetScroll = hoveredNodeIdx + 1;
                    boyMarching = false;
                }
                int totalLvls = std::max(1, gGameData.levelCount());
                int startIdx = (unit < 3) ? gGameData.unitStart(unit) : 0;
                selectedLevel = (startIdx + hoveredNodeIdx) % totalLvls;
                return selectedLevel;
            }
        }
        return -1;
    }

    void drawNodeIndicators(const std::vector<NodeDef>& nodes, float t) {
        if (nodes.empty()) return;
        int cur = activeNodeIdx % (int)nodes.size();
        Vector2 pos{ nodes[cur].x, nodes[cur].y };
        float pulse = 1.0f + 0.14f * std::sin(t * 5.2f);
        DrawCircleLines((int)pos.x, (int)pos.y, 25.0f * pulse, Color{ 253, 224, 71, 245 });
        DrawCircleLines((int)pos.x, (int)pos.y, 28.0f * pulse, Color{ 255, 255, 255, 195 });

        // Bouncing "PLAY" pointer badge above active node
        float bounce = std::sin(t * 6.0f) * 5.0f;
        Rectangle tag{ pos.x - 32.0f, pos.y - 54.0f + bounce, 64.0f, 22.0f };
        UI::GradientRound(tag, 8.0f, Color{ 16, 185, 129, 245 }, Color{ 5, 150, 105, 245 });
        UI::Rim(tag, 8.0f, Color{ 253, 224, 71, 255 }, WHITE, Color{ 6, 95, 70, 255 });
        Theme::text3D("PLAY", { pos.x, pos.y - 43.0f + bounce }, 12, WHITE, BLACK, 1.5f, BLACK, true, true);

        // Hover ring on any node under cursor
        if (hoveredNodeIdx >= 0 && hoveredNodeIdx < (int)nodes.size()) {
            Vector2 hp{ nodes[hoveredNodeIdx].x, nodes[hoveredNodeIdx].y };
            DrawCircle((int)hp.x, (int)hp.y, 26.0f, Color{ 255, 255, 255, 55 });
            DrawCircleLines((int)hp.x, (int)hp.y, 28.0f, WHITE);
        }
    }

    void drawGraduationAcademyMap(int w, int h) {
        float t = Theme::timeNow;

        if (!UIAssets::drawStretch("map_academy/map_background_clean.png",
                                   Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 56, 189, 248, 255 }, Color{ 186, 230, 253, 255 });
        }

        bool flagPose2 = ((int)(t * 4.5f) % 2) == 1;
        float flagTilt = std::sin(t * 5.0f) * 6.0f;
        const char* flagKey = flagPose2
            ? "map_academy/prop_castle_flag_pose2.png"
            : "map_academy/prop_castle_flag_pose1.png";
        UIAssets::drawCentered(flagKey, { 248, 74 }, 48, 48, flagTilt);
        UIAssets::drawCentered(flagKey, { 372, 74 }, 48, 48, -flagTilt);

        float globeTilt = std::sin(t * 2.8f) * 10.0f;
        UIAssets::drawCentered("map_academy/prop_globe_full.png",
                               { 156, 236 }, 88, 92, globeTilt);

        bool bookPose2 = ((int)(t * 5.2f) % 2) == 1;
        const char* bookKey = bookPose2
            ? "map_academy/prop_flying_book_pose2.png"
            : "map_academy/prop_flying_book_pose1.png";
        UIAssets::drawCentered(bookKey, { 105, 355 + std::sin(t * 4.0f) * 8.0f }, 68, 58, std::sin(t * 3.5f) * 8.0f);
        UIAssets::drawCentered(bookKey, { 445, 365 + std::cos(t * 3.8f) * 8.0f }, 66, 56, std::cos(t * 3.2f) * -8.0f);

        const auto& nodes = units[3].nodes;
        for (size_t i = 0; i + 1 < nodes.size(); ++i) {
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       7.0f, Color{ 251, 191, 36, 175 });
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       3.0f, Color{ 254, 249, 195, 235 });
        }

        for (size_t i = 0; i < nodes.size(); ++i) {
            char scrollKey[128];
            std::snprintf(scrollKey, sizeof(scrollKey),
                          "map_academy/scrolls/scroll_step_%02d.png",
                          (int)i + 1);
            bool isHov = ((int)i == hoveredNodeIdx);
            float pulse = ((int)i == (int)std::round(boyProgress)) ? (1.0f + 0.08f * std::sin(t * 5.0f)) : (isHov ? 1.12f : 1.0f);
            UIAssets::drawCentered(scrollKey, { nodes[i].x, nodes[i].y }, 82 * pulse, 62 * pulse);
        }

        bool owlFlap = ((int)(t * 4.8f) % 2) == 1;
        const char* owlKey = owlFlap
            ? "map_academy/char_owl_pose2_flapping.png"
            : "map_academy/char_owl_pose1_idle.png";
        UIAssets::drawCentered(owlKey, { 95, 815 + std::sin(t * 3.6f) * 6.0f }, 78, 86);

        int idxA = std::clamp((int)std::floor(boyProgress), 0, 13);
        int idxB = std::clamp(idxA + 1, 0, 13);
        float frac = boyProgress - (float)idxA;
        Vector2 boyPos{
            nodes[idxA].x + (nodes[idxB].x - nodes[idxA].x) * frac,
            nodes[idxA].y + (nodes[idxB].y - nodes[idxA].y) * frac - 32.0f
        };
        float hopY = std::abs(std::sin(t * 5.5f)) * -10.0f;
        bool boyJump = ((int)(t * 4.0f) % 2) == 1 || boyMarching;
        const char* boyKey = boyJump
            ? "map_academy/char_boy_pose2_jumping.png"
            : "map_academy/char_boy_pose1_cheering.png";
        UIAssets::drawCentered(boyKey, { boyPos.x, boyPos.y + hopY }, 92, 108);
    }

    void drawSafetyFoodIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawFrameSequence("map_safety_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            UIAssets::drawStretch("assets/scenery/clean_safety.png", Rectangle{ 0, 0, (float)w, (float)h });
        }
        drawNodeIndicators(units[1].nodes, t);
    }

    void drawCosmicSpaceIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawFrameSequence("map_space_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            UIAssets::drawStretch("assets/scenery/clean_space.png", Rectangle{ 0, 0, (float)w, (float)h });
        }
        drawNodeIndicators(units[2].nodes, t);
    }

    void drawEthicsIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawFrameSequence("map_ethics_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            UIAssets::drawStretch("assets/scenery/clean_ethics.png", Rectangle{ 0, 0, (float)w, (float)h });
        }
        drawNodeIndicators(units[0].nodes, t);
    }

    void draw(int w, int h) {
        if (unit == 0) drawEthicsIsland(w, h);
        else if (unit == 1) drawSafetyFoodIsland(w, h);
        else if (unit == 2) drawCosmicSpaceIsland(w, h);
        else drawGraduationAcademyMap(w, h);

        Vector2 m = Input::Mouse();

        // Top Unit Selector Bar
        const char* tabLabels[4] = { "1.ETHICS", "2.SAFETY", "3.SPACE", "4.ACADEMY" };
        for (int u = 0; u < 4; ++u) {
            Rectangle tabRec{ 10.0f + u * 131.0f, 10.0f, 125.0f, 36.0f };
            bool hov = CheckCollisionPointRec(m, tabRec);
            Color topC = (unit == u) ? Color{ 251, 191, 36, 245 } : (hov ? Color{ 51, 65, 85, 235 } : Color{ 15, 23, 42, 205 });
            Color botC = (unit == u) ? Color{ 217, 119, 6, 245 } : Color{ 30, 41, 59, 205 };
            UI::GradientRound(tabRec, 12.0f, topC, botC);
            UI::Rim(tabRec, 12.0f, WHITE, WHITE, Color{ 100, 116, 139, 220 });
            Theme::text3D(tabLabels[u], { tabRec.x + 62.5f, tabRec.y + 18.0f },
                          14, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        // Bottom Corner Navigation Pills
        struct NavPill {
            Rectangle r;
            const char* label;
            Color col;
        };
        NavPill pills[4] = {
            { { 10, 912, 92, 40 }, "HOME", Color{ 16, 185, 129, 240 } },
            { { 108, 912, 86, 40 }, "SHOP", Color{ 124, 58, 237, 240 } },
            { { 346, 912, 88, 40 }, "STREAK", Color{ 234, 88, 12, 240 } },
            { { 440, 912, 90, 40 }, (unit == 3) ? "GO 1->14" : "NEXT MAP", Color{ 37, 99, 235, 240 } }
        };
        for (int i = 0; i < 4; ++i) {
            Rectangle dr = pills[i].r;
            if (CheckCollisionPointRec(m, dr)) dr.y -= 2.0f;
            UI::DropShadow(dr, 14.0f, 3.0f, 0.35f, 2.0f);
            UI::GradientRound(dr, 14.0f, pills[i].col, Color{ 15, 23, 42, 240 });
            UI::Rim(dr, 14.0f, Color{ 251, 191, 36, 255 }, WHITE, Color{ 180, 83, 9, 255 });
            Theme::text3D(pills[i].label, { dr.x + dr.width * 0.5f, dr.y + 20.0f },
                          13, WHITE, BLACK, 2.0f, BLACK, true, true);
        }

        Theme::drawParticles();
    }
};
