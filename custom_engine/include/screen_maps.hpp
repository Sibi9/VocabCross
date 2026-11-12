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

    int unit = 0; // 0=Ethics, 1=Safety First, 2=Astronomy/Space, 3=Graduation Academy
    float scrollY = 0.0f;
    float targetScrollY = 0.0f;
    int selectedLevel = -1;
    int activeNodeIdx = 0;
    int hoveredNodeIdx = -1;
    int pendingNavScreen = -1;
    std::vector<UnitNodes> units;

    // Interactive spring physics for level nodes
    float nodeScale[16] = { 1.0f };
    float nodePressY[16] = { 0.0f };
    float nodeShake[16] = { 0.0f };

    // Landmark interactive animation timers
    float bellWobble = 0.0f;
    float scalesTilt = 0.0f;
    float detectiveWobble = 0.0f;
    float safeGlimmer = 0.0f;
    float signBounce = 0.0f;

    // Boy animation on Graduation Academy
    int boyTargetScroll = 4;
    float boyProgress = 3.0f;
    bool boyMarching = false;

    void init() {
        units.resize(4);

        for (int i = 0; i < 16; ++i) {
            nodeScale[i] = 1.0f;
            nodePressY[i] = 0.0f;
            nodeShake[i] = 0.0f;
        }

        // Exact pixel centers of the 10 nodes on Unit 0: Ethics Island (540x960)
        // Measured directly from the living animation frames to match the UI kit exactly
        units[0].nodes = {
            { 271.0f, 763.0f, 1 },
            { 346.0f, 743.0f, 2 },
            { 390.0f, 684.0f, 3 },
            { 345.0f, 626.0f, 4 },
            { 280.0f, 588.0f, 5 },
            { 236.0f, 535.0f, 6 },
            { 313.0f, 463.0f, 7 },
            { 263.0f, 393.0f, 8 },
            { 227.0f, 371.0f, 9 },
            { 250.0f, 318.0f, 10 }
        };
        units[0].hasSign = true;
        units[0].signX = 150.0f; units[0].signY = 815.0f; units[0].signW = 240.0f; units[0].signH = 115.0f;

        // Verified centers on Unit 1: Safety First Food Island (540x960)
        units[1].nodes = {
            { 276.0f, 892.0f, 1 }, { 378.0f, 830.0f, 2 }, { 404.0f, 774.0f, 3 }, { 355.0f, 725.0f, 4 }, { 268.0f, 566.0f, 5 },
            { 225.0f, 526.0f, 6 }, { 256.0f, 488.0f, 7 }, { 300.0f, 444.0f, 8 }, { 296.0f, 368.0f, 9 }, { 286.0f, 252.0f, 10 }
        };

        // Verified centers on Unit 2: Space Observatory Island (540x960)
        units[2].nodes = {
            { 258.0f, 822.0f, 1 }, { 138.0f, 728.0f, 2 }, { 248.0f, 654.0f, 3 }, { 372.0f, 588.0f, 4 }, { 402.0f, 495.0f, 5 },
            { 312.0f, 472.0f, 6 }, { 222.0f, 398.0f, 7 }, { 268.0f, 325.0f, 8 }, { 338.0f, 265.0f, 9 }, { 295.0f, 205.0f, 10 }
        };

        // Unit 3: Graduation Academy Map (14 Numbered Scrolls)
        units[3].nodes = {
            { 165.0f, 845.0f, 1 }, { 268.0f, 825.0f, 2 }, { 372.0f, 792.0f, 3 }, { 312.0f, 738.0f, 4 },
            { 195.0f, 702.0f, 5 }, { 152.0f, 642.0f, 6 }, { 248.0f, 598.0f, 7 }, { 358.0f, 560.0f, 8 },
            { 382.0f, 495.0f, 9 }, { 282.0f, 452.0f, 10 }, { 225.0f, 392.0f, 11 }, { 278.0f, 338.0f, 12 },
            { 335.0f, 282.0f, 13 }, { 285.0f, 222.0f, 14 }
        };
    }

    void setUnit(int u) {
        unit = std::clamp(u, 0, 3);
        selectedLevel = -1;
        activeNodeIdx = 0;
        hoveredNodeIdx = -1;
        for (int i = 0; i < 16; ++i) {
            nodeScale[i] = 1.0f;
            nodePressY[i] = 0.0f;
            nodeShake[i] = 0.0f;
        }
    }

    int update(float dt) {
        selectedLevel = -1;
        pendingNavScreen = -1;
        hoveredNodeIdx = -1;

        // Landmark animation decays
        if (bellWobble > 0.0f) bellWobble = std::max(0.0f, bellWobble - dt * 2.8f);
        if (scalesTilt > 0.0f) scalesTilt = std::max(0.0f, scalesTilt - dt * 2.5f);
        if (detectiveWobble > 0.0f) detectiveWobble = std::max(0.0f, detectiveWobble - dt * 3.0f);
        if (safeGlimmer > 0.0f) safeGlimmer = std::max(0.0f, safeGlimmer - dt * 3.0f);
        if (signBounce > 0.0f) signBounce = std::max(0.0f, signBounce - dt * 3.5f);

        // Academy boy marching logic
        if (boyMarching) {
            boyProgress += dt * 2.2f;
            if (boyProgress >= 13.0f) {
                boyProgress = 13.0f;
                boyMarching = false;
                gAudio.playFanfare();
                Theme::spawnCelebration({ 285, 200 }, 64);
            }
        } else if (unit == 3) {
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

        // Keyboard hotkeys for smooth navigation
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
            gAudio.playClick();
            pendingNavScreen = Nav::SCREEN_HOME;
            return -1;
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_PAGE_DOWN) || IsKeyPressed(KEY_D)) {
            setUnit((unit + 1) % 4);
            gAudio.playClick();
            return -1;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_PAGE_UP) || IsKeyPressed(KEY_A)) {
            setUnit((unit + 3) % 4);
            gAudio.playClick();
            return -1;
        }

        // --- Interactive Landmarks on Ethics Island (Unit 0) ---
        if (unit == 0) {
            // 1. Wooden Signpost ("ETHICS ISLAND") -> Return to Home
            Rectangle signRect{ units[0].signX, units[0].signY, units[0].signW, units[0].signH };
            if (CheckCollisionPointRec(m, signRect)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if (mouseReleased) {
                    signBounce = 1.0f;
                    gAudio.playClick();
                    pendingNavScreen = Nav::SCREEN_HOME;
                    return -1;
                }
            }

            // 2. Golden Herald Bell ({ 435, 550 }, radius 38)
            Vector2 bellCenter{ 435.0f, 550.0f };
            if (CheckCollisionPointCircle(m, bellCenter, 38.0f)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if (mouseReleased) {
                    bellWobble = 1.0f;
                    gAudio.playClick();
                    Theme::spawnCelebration({ bellCenter.x, bellCenter.y - 12.0f }, 16);
                }
            }

            // 3. Auditor Detective Mascot ({ 110, 635 }, radius 46)
            Vector2 detectiveCenter{ 110.0f, 635.0f };
            if (CheckCollisionPointCircle(m, detectiveCenter, 46.0f)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if (mouseReleased) {
                    detectiveWobble = 1.0f;
                    gAudio.playPop();
                    Theme::spawnCelebration({ 145.0f, 605.0f }, 14);
                }
            }

            // 4. Scales of Justice & Temple ({ 385, 200 }, radius 56)
            Vector2 scalesCenter{ 385.0f, 200.0f };
            if (CheckCollisionPointCircle(m, scalesCenter, 56.0f)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if (mouseReleased) {
                    scalesTilt = 1.0f;
                    gAudio.playClick();
                    Theme::spawnCelebration(scalesCenter, 18);
                }
            }

            // 5. Glass Vault Safe ({ 145, 425 }, radius 42)
            Vector2 safeCenter{ 145.0f, 425.0f };
            if (CheckCollisionPointCircle(m, safeCenter, 42.0f)) {
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if (mouseReleased) {
                    safeGlimmer = 1.0f;
                    gAudio.playPop();
                    Theme::spawnCelebration(safeCenter, 16);
                }
            }
        }

        // --- Level Nodes Interaction with 4-State Spring Physics ---
        const auto& nodes = units[unit].nodes;
        int startIdx = gGameData.unitStart(unit);
        int count = gGameData.unitLevelCount(unit);

        for (size_t i = 0; i < nodes.size(); ++i) {
            Vector2 nodePos = { nodes[i].x, nodes[i].y };
            int lvlIdx = startIdx + (count > 0 ? ((int)i % count) : 0);
            bool isUnlocked = (lvlIdx >= 0 && lvlIdx < gGameData.levelCount())
                ? gGameData.levels[lvlIdx].unlocked
                : (i == 0);

            // Shake decay for locked feedback
            if (nodeShake[i] > 0.0f) {
                nodeShake[i] = std::max(0.0f, nodeShake[i] - dt);
            }

            float hitRadius = 26.0f;
            bool isHovered = CheckCollisionPointCircle(m, nodePos, hitRadius);
            bool isPressed = isHovered && mouseDown;

            if (isHovered) {
                hoveredNodeIdx = (int)i;
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            }

            // Smooth spring scaling physics
            float targetScale = 1.0f;
            float targetPressY = 0.0f;
            if (isPressed) {
                targetScale = 0.95f;
                targetPressY = 3.2f;
            } else if (isHovered) {
                targetScale = 1.12f;
                targetPressY = -2.0f;
            }
            nodeScale[i] += (targetScale - nodeScale[i]) * std::min(1.0f, 15.0f * dt);
            nodePressY[i] += (targetPressY - nodePressY[i]) * std::min(1.0f, 15.0f * dt);

            if (isHovered && mouseReleased) {
                if (isUnlocked) {
                    gAudio.playPop();
                    Theme::spawnCelebration(nodePos, 32);
                    activeNodeIdx = (int)i;
                    if (unit == 3) {
                        boyTargetScroll = (int)i + 1;
                        boyMarching = false;
                    }
                    selectedLevel = lvlIdx;
                    return selectedLevel;
                } else {
                    // Locked level feedback: tactile shake and click sound
                    gAudio.playClick();
                    nodeShake[i] = 0.32f;
                }
            }
        }

        return -1;
    }

    void drawEthicsIsland(int w, int h) {
        float t = Theme::timeNow;

        // 1. Living Animated Ethics Island background (16 Frames Sequence at 10 FPS)
        if (!UIAssets::drawFrameSequence("map_ethics_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            UIAssets::drawStretch("assets/scenery/clean_ethics.png", Rectangle{ 0, 0, (float)w, (float)h });
        }

        // 2. Active Stage Beacon & Continuous Pulse Animation
        int activeIdx = std::clamp(activeNodeIdx, 0, (int)units[0].nodes.size() - 1);
        Vector2 activePos = { units[0].nodes[activeIdx].x, units[0].nodes[activeIdx].y };

        float pulse = 1.0f + 0.045f * std::sin(t * 4.8f);
        float auraAlpha = 0.55f + 0.35f * std::sin(t * 4.8f);

        // Radiant cyan halo around the playable stage
        DrawCircleLines((int)activePos.x, (int)activePos.y, 25.0f * pulse + 3.0f,
                        Theme::WithAlpha(Color{ 56, 189, 248, 255 }, auraAlpha));
        DrawCircleLines((int)activePos.x, (int)activePos.y, 25.0f * pulse + 5.5f,
                        Theme::WithAlpha(Color{ 186, 230, 253, 255 }, auraAlpha * 0.45f));

        // Ambient upward sparkle beacons
        if (((int)(t * 6.0f) % 5) == 0) {
            float sparkAngle = (float)((int)(t * 20.0f) % 360) * DEG2RAD;
            float sparkDist = 18.0f + 10.0f * std::sin(t * 7.0f);
            Vector2 sparkPos{ activePos.x + std::cos(sparkAngle) * sparkDist,
                              activePos.y + std::sin(sparkAngle) * sparkDist - 6.0f };
            DrawCircleV(sparkPos, 2.0f, Theme::WithAlpha(Color{ 254, 240, 138, 255 }, auraAlpha));
        }

        // 3. Dynamic Hover / Press Spring Overlay from ethics_island_ui_kit
        for (size_t i = 0; i < units[0].nodes.size(); ++i) {
            float s = nodeScale[i];
            float py = nodePressY[i];
            float shake = (nodeShake[i] > 0.0f) ? std::sin(nodeShake[i] * 42.0f) * 4.0f : 0.0f;
            Vector2 pos = { units[0].nodes[i].x + shake, units[0].nodes[i].y + py };
            int lvlNum = units[0].nodes[i].number;
            bool isHov = ((int)i == hoveredNodeIdx);

            // Only overlay on interaction or state scaling so the living frame art remains 100% authentic
            if (isHov || s > 1.02f || py > 0.5f || shake != 0.0f) {
                char nodeKey[128];
                if (lvlNum <= 8) {
                    std::snprintf(nodeKey, sizeof(nodeKey),
                                  "ethics_island_ui_kit/level_nodes/level_node_%d.png", lvlNum);
                } else {
                    std::snprintf(nodeKey, sizeof(nodeKey),
                                  "ethics_island_ui_kit/level_nodes/node_blank_stone.png");
                }

                float baseSize = 56.0f;
                float drawSize = baseSize * s;

                // Soft drop shadow under the floating node
                DrawCircle((int)pos.x, (int)(pos.y + 4.5f * s), 22.0f * s, Theme::WithAlpha(BLACK, 0.35f));

                if (!UIAssets::drawCentered(nodeKey, pos, drawSize, drawSize)) {
                    char fallbackKey[64];
                    std::snprintf(fallbackKey, sizeof(fallbackKey), "level_node_%d.png", lvlNum);
                    UIAssets::drawCentered(fallbackKey, pos, drawSize, drawSize);
                }

                // Interactive golden glint sweep
                if (isHov) {
                    float glintT = std::fmod(t * 3.5f, 1.0f);
                    float glintX = pos.x - 20.0f + glintT * 40.0f;
                    DrawLineEx({ glintX, pos.y - 12.0f }, { glintX + 8.0f, pos.y + 12.0f },
                               2.2f, Theme::WithAlpha(WHITE, 0.75f));
                }
            }
        }

        // 4. Interactive Landmark Visual Overlays
        // Golden Bell wobble
        if (bellWobble > 0.01f) {
            float bellAngle = std::sin(bellWobble * 24.0f) * 8.0f;
            UIAssets::drawCentered("ethics_island_ui_kit/ui_elements/golden_bell.png",
                                   { 435.0f, 550.0f }, 72.0f, 82.0f, bellAngle);
        }

        // Scales of Justice gentle sway
        float breezeTilt = std::sin(t * 2.2f) * 3.0f;
        if (scalesTilt > 0.01f) {
            breezeTilt += std::sin(scalesTilt * 20.0f) * 7.0f;
            UIAssets::drawCentered("ethics_island_ui_kit/ui_elements/scales_of_justice.png",
                                   { 385.0f, 200.0f }, 110.0f, 102.0f, breezeTilt);
        }

        // Wooden Signpost subtle spring
        if (signBounce > 0.01f) {
            float signScale = 1.0f + 0.05f * std::sin(signBounce * 16.0f);
            UIAssets::drawCentered("ethics_island_ui_kit/ui_elements/signpost_ethics_island.png",
                                   { 270.0f, 872.0f }, 240.0f * signScale, 115.0f * signScale);
        }
    }

    void drawSafetyFoodIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawFrameSequence("map_safety_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            UIAssets::drawStretch("assets/scenery/clean_safety.png", Rectangle{ 0, 0, (float)w, (float)h });
        }
        drawCleanNodes(units[1].nodes, t);
    }

    void drawCosmicSpaceIsland(int w, int h) {
        float t = Theme::timeNow;
        if (!UIAssets::drawFrameSequence("map_space_living/frame_", 16, t, 10.0f,
                                         Rectangle{ 0, 0, (float)w, (float)h }, WHITE, true)) {
            UIAssets::drawStretch("assets/scenery/clean_space.png", Rectangle{ 0, 0, (float)w, (float)h });
        }
        drawCleanNodes(units[2].nodes, t);
    }

    void drawCleanNodes(const std::vector<NodeDef>& nodes, float t) {
        if (nodes.empty()) return;
        int activeIdx = std::clamp(activeNodeIdx, 0, (int)nodes.size() - 1);
        Vector2 activePos = { nodes[activeIdx].x, nodes[activeIdx].y };
        float pulse = 1.0f + 0.045f * std::sin(t * 4.8f);
        DrawCircleLines((int)activePos.x, (int)activePos.y, 24.0f * pulse + 3.0f,
                        Theme::WithAlpha(Theme::gold, 0.70f));
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
        for (size_t i = 0; i < nodes.size(); ++i) {
            char scrollKey[128];
            std::snprintf(scrollKey, sizeof(scrollKey),
                          "map_academy/scrolls/scroll_step_%02d.png",
                          (int)i + 1);
            bool isHov = ((int)i == hoveredNodeIdx);
            float pulse = ((int)i == (int)std::round(boyProgress))
                ? (1.0f + 0.08f * std::sin(t * 5.0f))
                : (isHov ? 1.12f : 1.0f);
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

    void draw(int w, int h) {
        if (unit == 0) drawEthicsIsland(w, h);
        else if (unit == 1) drawSafetyFoodIsland(w, h);
        else if (unit == 2) drawCosmicSpaceIsland(w, h);
        else drawGraduationAcademyMap(w, h);

        Theme::drawParticles();
    }
};
