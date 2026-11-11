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

    UI::StatefulNodeButton nodeWidgets[16];
    UI::StatefulButton tabWidgets[4];
    UI::StatefulButton navPills[4];

    int boyTargetScroll = 4;
    float boyProgress = 3.0f;
    bool boyMarching = false;

    void init() {
        units.resize(4);

        const char* tabLabels[4] = { "1.ETHICS", "2.SAFETY", "3.SPACE", "4.ACADEMY" };
        for (int u = 0; u < 4; ++u) {
            tabWidgets[u].bounds = Rectangle{ 10.0f + u * 131.0f, 8.0f, 125.0f, 38.0f };
            tabWidgets[u].label = tabLabels[u];
            tabWidgets[u].cornerRadius = 10.0f;
            tabWidgets[u].fontSize = 13.0f;
            tabWidgets[u].topColor = Color{ 30, 41, 59, 230 };
            tabWidgets[u].bottomColor = Color{ 15, 23, 42, 230 };
            tabWidgets[u].lipColor = Color{ 2, 6, 23, 230 };
            tabWidgets[u].borderColor = Color{ 148, 163, 184, 255 };
        }

        navPills[0].bounds = Rectangle{ 10, 910, 92, 42 };
        navPills[0].label = "HOME";
        navPills[0].topColor = Color{ 16, 185, 129, 240 };
        navPills[0].bottomColor = Color{ 5, 150, 105, 240 };
        navPills[0].lipColor = Color{ 6, 95, 70, 240 };
        navPills[0].borderColor = Color{ 167, 243, 208, 255 };
        navPills[0].cornerRadius = 14.0f;
        navPills[0].fontSize = 13.0f;

        navPills[1].bounds = Rectangle{ 108, 910, 90, 42 };
        navPills[1].label = "SHOP";
        navPills[1].topColor = Color{ 124, 58, 237, 240 };
        navPills[1].bottomColor = Color{ 76, 29, 149, 240 };
        navPills[1].lipColor = Color{ 59, 7, 100, 240 };
        navPills[1].borderColor = Color{ 216, 180, 254, 255 };
        navPills[1].cornerRadius = 14.0f;
        navPills[1].fontSize = 13.0f;

        navPills[2].bounds = Rectangle{ 342, 910, 92, 42 };
        navPills[2].label = "STREAK";
        navPills[2].topColor = Color{ 234, 88, 12, 240 };
        navPills[2].bottomColor = Color{ 154, 52, 18, 240 };
        navPills[2].lipColor = Color{ 124, 45, 18, 240 };
        navPills[2].borderColor = Color{ 254, 215, 170, 255 };
        navPills[2].cornerRadius = 14.0f;
        navPills[2].fontSize = 13.0f;

        navPills[3].bounds = Rectangle{ 438, 910, 94, 42 };
        navPills[3].label = "NEXT MAP";
        navPills[3].topColor = Color{ 37, 99, 235, 240 };
        navPills[3].bottomColor = Color{ 30, 58, 138, 240 };
        navPills[3].lipColor = Color{ 23, 37, 84, 240 };
        navPills[3].borderColor = Color{ 191, 219, 254, 255 };
        navPills[3].cornerRadius = 14.0f;
        navPills[3].fontSize = 13.0f;

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
        bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        // 1. Top Unit Selector Tabs (4 Stateful Tabs)
        for (int u = 0; u < 4; ++u) {
            if (unit == u) {
                tabWidgets[u].topColor = Color{ 245, 158, 11, 255 };
                tabWidgets[u].bottomColor = Color{ 180, 83, 9, 255 };
                tabWidgets[u].lipColor = Color{ 120, 53, 15, 255 };
                tabWidgets[u].borderColor = Color{ 254, 240, 138, 255 };
            } else {
                tabWidgets[u].topColor = Color{ 30, 41, 59, 230 };
                tabWidgets[u].bottomColor = Color{ 15, 23, 42, 230 };
                tabWidgets[u].lipColor = Color{ 2, 6, 23, 230 };
                tabWidgets[u].borderColor = Color{ 148, 163, 184, 255 };
            }
            tabWidgets[u].update(m, mouseDown, dt);
            if (tabWidgets[u].isClicked(m, mouseReleased)) {
                setUnit(u);
                gAudio.playClick();
                return -1;
            }
        }

        // 2. Bottom Navigation Pills (4 Stateful Action Buttons)
        navPills[3].label = (unit == 3) ? "MARCH" : "NEXT MAP";
        for (int i = 0; i < 4; ++i) {
            navPills[i].update(m, mouseDown, dt);
        }
        if (navPills[0].isClicked(m, mouseReleased)) {
            gAudio.playClick();
            pendingNavScreen = Nav::SCREEN_HOME;
            return -1;
        }
        if (navPills[1].isClicked(m, mouseReleased)) {
            gAudio.playClick();
            pendingNavScreen = Nav::SCREEN_SHOP;
            return -1;
        }
        if (navPills[2].isClicked(m, mouseReleased)) {
            gAudio.playClick();
            pendingNavScreen = Nav::SCREEN_STREAK;
            return -1;
        }
        if (navPills[3].isClicked(m, mouseReleased)) {
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

        // 3. Level Nodes (Stateful 4-State Interactive Widgets)
        const auto& nodes = units[unit].nodes;
        int startIdx = gGameData.unitStart(unit);
        int count = gGameData.unitLevelCount(unit);

        for (size_t i = 0; i < nodes.size(); ++i) {
            nodeWidgets[i].center = { nodes[i].x, nodes[i].y };
            nodeWidgets[i].levelNumber = (int)i + 1;
            nodeWidgets[i].active = ((int)i == activeNodeIdx);
            int lvlIdx = startIdx + (count > 0 ? ((int)i % count) : 0);
            if (lvlIdx >= 0 && lvlIdx < gGameData.levelCount()) {
                nodeWidgets[i].unlocked = gGameData.levels[lvlIdx].unlocked;
                nodeWidgets[i].stars = gGameData.levels[lvlIdx].stars;
            } else {
                nodeWidgets[i].unlocked = false;
                nodeWidgets[i].stars = 0;
            }
            nodeWidgets[i].update(m, mouseDown, dt);
            if (nodeWidgets[i].isHovered) {
                hoveredNodeIdx = (int)i;
            }
            if (nodeWidgets[i].isClicked(m, mouseReleased)) {
                gAudio.playPop();
                activeNodeIdx = (int)i;
                if (unit == 3) {
                    boyTargetScroll = (int)i + 1;
                    boyMarching = false;
                }
                selectedLevel = lvlIdx;
                return selectedLevel;
            }
        }

        return -1;
    }

    void drawNodeIndicators(const std::vector<NodeDef>& nodes, float t) {
        if (nodes.empty()) return;

        // Clean connecting path
        for (size_t i = 0; i + 1 < nodes.size(); ++i) {
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       8.0f, Theme::WithAlpha(Color{ 180, 83, 9, 255 }, 0.50f));
            DrawLineEx({ nodes[i].x, nodes[i].y }, { nodes[i + 1].x, nodes[i + 1].y },
                       4.0f, Theme::WithAlpha(Color{ 254, 240, 138, 255 }, 0.85f));
        }

        // Draw stateful node widgets (Zero circular overlays)
        for (size_t i = 0; i < nodes.size(); ++i) {
            nodeWidgets[i].draw();
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

        // Top Unit Selector Bar (4 Stateful Tabs)
        for (int u = 0; u < 4; ++u) {
            tabWidgets[u].draw();
        }

        // Bottom Navigation Pills (4 Stateful Buttons)
        navPills[3].label = (unit == 3) ? "GO 1->14" : "NEXT MAP";
        for (int i = 0; i < 4; ++i) {
            navPills[i].draw();
        }

        Theme::drawParticles();
    }
};
