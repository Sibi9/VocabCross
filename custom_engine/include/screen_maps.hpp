#pragma once
#include "raylib.h"
#include "input.hpp"
#include "audio_manager.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "theme.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
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

    int unit = 0;
    float time = 0.0f;
    bool selectorOpen = false;
    bool settingsOpen = false;
    bool verifyAllUnlocked = false;
    float toastTimer = 0.0f;
    std::string toast;

    Texture2D cleanBg[3]{};
    std::vector<UnitNodes> unitNodes;
    Vector2 lastMouse{};

    void init() {
        cleanBg[0] = LoadTexture("assets/scenery/clean_ethics.png");
        cleanBg[1] = LoadTexture("assets/scenery/clean_safety.png");
        cleanBg[2] = LoadTexture("assets/scenery/clean_space.png");
        for (int i = 0; i < 3; ++i) SetTextureFilter(cleanBg[i], TEXTURE_FILTER_BILINEAR);

        unitNodes.resize(4);
        // Ethics island: 8 reference nodes + 2 upper temple nodes for the 10 levels.
        unitNodes[0].nodes = {
            { 270, 768, 1 }, { 348, 742, 2 }, { 384, 680, 3 }, { 338, 618, 4 }, { 280, 590, 5 },
            { 228, 548, 6 }, { 318, 468, 7 }, { 262, 420, 8 }, { 250, 365, 9 }, { 286, 335, 10 },
        };
        unitNodes[0].hasSign = true;
        unitNodes[0].signX = 158; unitNodes[0].signY = 822; unitNodes[0].signW = 244; unitNodes[0].signH = 108;

        // Safety island has 8 curriculum levels (the plan's Unit 2 count).
        unitNodes[1].nodes = {
            { 267, 858, 1 }, { 363, 787, 2 }, { 339, 713, 3 }, { 330, 640, 4 },
            { 247, 590, 5 }, { 217, 530, 6 }, { 238, 462, 7 }, { 278, 408, 8 },
        };
        unitNodes[1].hasSign = false;

        // Space island: 7 reference nodes + node 08 on the launch platform.
        unitNodes[2].nodes = {
            { 268, 835, 1 }, { 160, 712, 2 }, { 270, 615, 3 }, { 370, 538, 4 },
            { 396, 438, 5 }, { 309, 398, 6 }, { 232, 318, 7 }, { 296, 280, 8 },
        };
        unitNodes[2].hasSign = false;

        // Emotions island (original, procedural scene).
        unitNodes[3].nodes = {
            { 270, 815, 1 }, { 330, 760, 2 }, { 320, 685, 3 }, { 240, 625, 4 }, { 205, 545, 5 }, { 270, 480, 6 },
        };
        unitNodes[3].hasSign = true;
        unitNodes[3].signX = 148; unitNodes[3].signY = 840; unitNodes[3].signW = 244; unitNodes[3].signH = 78;

        unit = 0;
        gGameData.current_level = 0;
    }

    void unload() {
        for (int i = 0; i < 3; ++i) {
            if (cleanBg[i].id > 0) UnloadTexture(cleanBg[i]);
        }
    }

    void setUnit(int u) {
        unit = std::clamp(u, 0, 3);
        int start = gGameData.unitStart(unit);
        int best = start;
        for (int i = start; i < start + gGameData.unitLevelCount(unit); ++i) {
            if (gGameData.levels[i].unlocked) best = i;
        }
        gGameData.current_level = best;
    }

    int starsInUnit(int u) const {
        int start = gGameData.unitStart(u);
        int total = 0;
        for (int i = start; i < start + gGameData.unitLevelCount(u); ++i) total += gGameData.levels[i].stars;
        return total;
    }

    void showToast(const std::string& msg) {
        toast = msg;
        toastTimer = 2.2f;
    }

    void update(float dt) {
        time += dt;
        if (toastTimer > 0.0f) toastTimer -= dt;
    }

    UI::MapNodeStyle styleForUnit() const {
        UI::MapNodeStyle s;
        if (unit == 0) {
            s.faceA = Color{ 56, 189, 248, 255 };
            s.faceB = Color{ 2, 132, 199, 255 };
            s.lip = Color{ 3, 105, 161, 255 };
            s.ring = Color{ 224, 242, 254, 255 };
        } else if (unit == 1) {
            s.faceA = Color{ 251, 191, 36, 255 };
            s.faceB = Color{ 217, 119, 6, 255 };
            s.lip = Color{ 146, 64, 14, 255 };
            s.ring = Color{ 254, 243, 199, 255 };
        } else if (unit == 2) {
            s.faceA = Color{ 56, 189, 248, 255 };
            s.faceB = Color{ 2, 132, 199, 255 };
            s.lip = Color{ 3, 105, 161, 255 };
            s.ring = Color{ 224, 242, 254, 255 };
            s.lockedA = Color{ 125, 211, 252, 255 };
            s.lockedB = Color{ 30, 64, 175, 255 };
            s.lockBelow = true;
        } else {
            s.faceA = Color{ 244, 114, 182, 255 };
            s.faceB = Color{ 190, 24, 93, 255 };
            s.lip = Color{ 136, 19, 55, 255 };
            s.ring = Color{ 252, 231, 243, 255 };
        }
        return s;
    }

    bool pointInNode(Vector2 p, const NodeDef& n) const {
        return CheckCollisionPointCircle(p, { n.x, n.y }, 32.0f);
    }

    void drawTopChip() {
        Rectangle chip{ 132.0f, 12.0f, 276.0f, 38.0f };
        UI::Capsule(chip, Color{ 30, 43, 68, 235 }, Color{ 13, 22, 40, 245 }, Theme::WithAlpha(Theme::gold, 0.85f));
        Theme::text3D(Theme::fontCartoon, gGameData.islandTitle(unit),
                      { chip.x + chip.width * 0.5f, chip.y + chip.height * 0.5f + 1.0f }, 15.0f,
                      Theme::goldLight, Color{ 76, 40, 20, 255 }, 1.4f, Theme::WithAlpha(BLACK, 0.5f), true, true);
        // chevrons
        for (int side = -1; side <= 1; side += 2) {
            Vector2 c{ side < 0 ? chip.x + 18.0f : chip.x + chip.width - 18.0f, chip.y + chip.height * 0.5f };
            Vector2 p1{ c.x + side * 5.0f, c.y - 7.0f };
            Vector2 p2{ c.x - side * 4.0f, c.y };
            Vector2 p3{ c.x + side * 5.0f, c.y + 7.0f };
            DrawLineEx(p1, p2, 3.0f, Theme::goldLight);
            DrawLineEx(p2, p3, 3.0f, Theme::goldLight);
        }
    }

    void drawSpaceHUD() {
        Rectangle capsule{ 14.0f, 14.0f, 276.0f, 48.0f };
        UI::Capsule(capsule, Color{ 30, 43, 68, 245 }, Color{ 12, 20, 38, 250 }, Color{ 96, 165, 250, 220 });
        Vector2 starC{ 40.0f, 38.0f };
        UI::Star5(starC, 27.0f, 13.0f, Theme::gold, Color{ 146, 64, 14, 255 }, true);
        Theme::text3D(Theme::fontCartoon, std::to_string(unit + 1), { starC.x, starC.y + 2.0f }, 24.0f,
                      WHITE, Color{ 146, 64, 14, 255 }, 1.8f, Theme::WithAlpha(BLACK, 0.5f), true, true);
        Rectangle track{ 67.0f, 25.0f, 207.0f, 26.0f };
        DrawRectangleRounded(track, 0.9f, 12, Color{ 15, 23, 42, 255 });
        int start = gGameData.unitStart(unit);
        int totalStars = std::max(1, gGameData.unitLevelCount(unit) * 3);
        float prog = std::clamp((float)starsInUnit(unit) / (float)totalStars, 0.10f, 1.0f);
        Rectangle fill{ track.x + 2.0f, track.y + 2.0f, (track.width - 4.0f) * prog, track.height - 4.0f };
        UI::GradientRound(fill, 12.0f, Color{ 125, 211, 252, 255 }, Color{ 14, 165, 233, 255 });
        DrawRectangleRounded({ fill.x + 3.0f, fill.y + 2.0f, std::max(1.0f, fill.width - 6.0f), fill.height * 0.35f },
                             0.8f, 6, Theme::WithAlpha(WHITE, 0.4f));
        (void)start;

        UI::CoinsBadge({ 318.0f, 17.0f }, 300, 140.0f, 42.0f);
        Rectangle menu{ 476.0f, 13.0f, 48.0f, 48.0f };
        UI::IconButton3D(menu, Color{ 56, 189, 248, 255 }, Color{ 2, 132, 199, 255 }, Color{ 186, 230, 253, 255 });
        UI::MenuIcon({ menu.x + 24.0f, menu.y + 24.0f }, 15.0f, WHITE);

        Rectangle gear{ 15.0f, 872.0f, 52.0f, 52.0f };
        UI::IconButton3D(gear, Color{ 56, 189, 248, 255 }, Color{ 3, 105, 161, 255 }, Color{ 186, 230, 253, 255 });
        UI::GearIcon({ gear.x + 26.0f, gear.y + 26.0f }, 15.0f, WHITE);
        Rectangle map{ 473.0f, 872.0f, 52.0f, 52.0f };
        UI::IconButton3D(map, Color{ 96, 165, 250, 255 }, Color{ 37, 99, 235, 255 }, Color{ 191, 219, 254, 255 });
        UI::MapPinIcon({ map.x + 26.0f, map.y + 23.0f }, 14.0f, Color{ 239, 68, 68, 255 });
    }

    void drawBottomButtons() {
        Rectangle gear{ 15.0f, 894.0f, 50.0f, 50.0f };
        UI::IconButton3D(gear, Color{ 56, 189, 248, 255 }, Color{ 3, 105, 161, 255 }, Color{ 186, 230, 253, 255 });
        UI::GearIcon({ gear.x + 25.0f, gear.y + 25.0f }, 14.0f, WHITE);

        Rectangle map{ 475.0f, 894.0f, 50.0f, 50.0f };
        UI::IconButton3D(map, Color{ 96, 165, 250, 255 }, Color{ 37, 99, 235, 255 }, Color{ 191, 219, 254, 255 });
        UI::MapPinIcon({ map.x + 25.0f, map.y + 22.0f }, 13.0f, Color{ 239, 68, 68, 255 });
    }

    void drawSigns() {
        if (!unitNodes[unit].hasSign) return;
        auto& s = unitNodes[unit];
        Rectangle board{ s.signX, s.signY, s.signW, s.signH };
        if (unit == 0) {
            UI::WoodenSign(board, "ETHICS", "ISLAND",
                           { Color{ 244, 114, 182, 255 }, Color{ 251, 146, 60, 255 }, Color{ 253, 224, 71, 255 },
                             Color{ 74, 222, 128, 255 }, Color{ 96, 165, 250, 255 }, Color{ 192, 132, 252, 255 } });
        } else if (unit == 3) {
            UI::WoodenSign(board, "EMOTIONS", "ISLAND",
                           { Color{ 253, 224, 71, 255 }, Color{ 244, 114, 182, 255 }, Color{ 125, 211, 252, 255 },
                             Color{ 253, 186, 116, 255 }, Color{ 216, 180, 254, 255 }, Color{ 134, 239, 172, 255 } });
        }
    }

    void drawNodes() {
        UI::MapNodeStyle style = styleForUnit();
        auto& defs = unitNodes[unit].nodes;
        int start = gGameData.unitStart(unit);
        Vector2 mouse = Input::Mouse();

        std::vector<const NodeDef*> ordered;
        for (const auto& n : defs) ordered.push_back(&n);
        std::sort(ordered.begin(), ordered.end(), [](const NodeDef* a, const NodeDef* b) { return a->y < b->y; });

        for (const NodeDef* np : ordered) {
            int gi = start + np->number - 1;
            if (gi < 0 || gi >= gGameData.levelCount()) continue;
            auto& lvl = gGameData.levels[gi];
            bool unlocked = lvl.unlocked || verifyAllUnlocked;
            bool current = (gi == gGameData.current_level);
            bool check = lvl.stars > 0 && np->number != 1 && !current;
            bool twoDigits = (unit == 2);
            bool hover = unlocked && CheckCollisionPointCircle(mouse, { np->x, np->y }, 30.0f);

            if (current) {
                float pulse = 0.5f + 0.5f * std::sin(time * 4.0f);
                DrawCircleV({ np->x, np->y }, 37.0f + pulse * 3.0f,
                            Theme::WithAlpha(style.faceA, 0.16f));
                DrawCircleLines((int)np->x, (int)np->y, 34.0f + pulse * 2.0f,
                                Theme::WithAlpha(WHITE, 0.30f + pulse * 0.25f));
            }
            if (hover) {
                DrawCircleV({ np->x, np->y }, 38.0f, Theme::WithAlpha(WHITE, 0.12f));
            }

            UI::MapNode({ np->x, np->y }, current ? 28.0f : 26.0f, np->number, unlocked, current,
                        lvl.stars, check, twoDigits, style);

            if (unit == 2 && np->number == 1 && lvl.stars > 0) {
                Theme::text3D(Theme::fontCartoon, "Cleared", { np->x, np->y + 58.0f }, 15.0f,
                              Theme::goldLight, Color{ 76, 40, 20, 255 }, 1.8f, Theme::WithAlpha(BLACK, 0.65f), true, true);
            }
        }
    }

    void drawSafetyFoliage() {
        // These two bushes cover the inpainting footprints left by the removed
        // reference HUD markers at the top of the food pyramid.
        auto bush = [&](Vector2 c, float s, Color leaf, Color dark) {
            DrawEllipse((int)c.x, (int)(c.y + 10.0f * s), 26.0f * s, 8.0f * s, Theme::WithAlpha(BLACK, 0.20f));
            DrawCircleV({ c.x - 12.0f * s, c.y }, 14.0f * s, dark);
            DrawCircleV({ c.x + 12.0f * s, c.y + 1.0f * s }, 13.0f * s, dark);
            DrawCircleV({ c.x, c.y - 8.0f * s }, 17.0f * s, leaf);
            DrawCircleV({ c.x - 10.0f * s, c.y - 2.0f * s }, 12.0f * s, leaf);
            DrawCircleV({ c.x + 10.0f * s, c.y - 1.0f * s }, 11.0f * s, leaf);
            DrawCircleV({ c.x - 5.0f * s, c.y - 13.0f * s }, 6.0f * s, Theme::WithAlpha(Color{ 190, 242, 100, 255 }, 0.65f));
            for (int i = 0; i < 3; ++i) {
                DrawCircleV({ c.x - 12.0f * s + i * 12.0f * s, c.y - 5.0f * s + (i % 2) * 5.0f * s },
                            2.2f * s, (i % 2 == 0) ? Color{ 251, 191, 36, 255 } : Color{ 244, 114, 182, 255 });
            }
        };
        bush({ 272.0f, 344.0f }, 0.95f, Color{ 34, 197, 94, 255 }, Color{ 21, 128, 61, 255 });
        bush({ 273.0f, 247.0f }, 0.82f, Color{ 74, 222, 128, 255 }, Color{ 22, 163, 74, 255 });
    }

    void drawToast() {
        if (toastTimer <= 0.0f) return;
        float a = std::min(1.0f, toastTimer / 0.4f);
        Rectangle r{ 90.0f, 64.0f, 360.0f, 40.0f };
        DrawRectangleRounded(r, 0.5f, 10, Theme::WithAlpha(Color{ 15, 23, 42, 240 }, a));
        DrawRectangleRoundedLines(r, 0.5f, 10, 1.4f, Theme::WithAlpha(Theme::gold, a * 0.8f));
        Theme::text(toast, { r.x + r.width * 0.5f, r.y + 11.0f }, 13.0f,
                    Theme::WithAlpha(WHITE, a), true);
    }

    void drawSelector() {
        DrawRectangle(0, 0, 540, 960, Theme::WithAlpha(BLACK, 0.62f));
        Rectangle panel{ 28.0f, 84.0f, 484.0f, 792.0f };
        UI::DropShadow(panel, 26.0f, 8.0f, 0.45f, 3.0f);
        UI::GradientRound(panel, 26.0f, Color{ 24, 37, 64, 250 }, Color{ 9, 15, 30, 252 });
        DrawRectangleRoundedLines(panel, UI::Roundness(panel, 26.0f), 16, 2.5f, Theme::WithAlpha(Theme::gold, 0.9f));
        Theme::text3D(Theme::fontCartoon, "SELECT ISLAND", { 270.0f, 120.0f }, 27.0f,
                      Theme::goldLight, Color{ 76, 40, 20, 255 }, 2.0f, Theme::WithAlpha(BLACK, 0.55f), true, true);

        const char* names[] = { "ETHICS ISLAND", "SAFETY ISLAND", "SPACE ISLAND", "EMOTIONS ISLAND" };
        const char* subs[] = { "Ethics in Business", "Safety First", "Astronomy & Space", "Feelings and Humor" };
        Vector2 mouse = Input::Mouse();
        for (int i = 0; i < 4; ++i) {
            Rectangle card{ 52.0f, 168.0f + i * 166.0f, 436.0f, 142.0f };
            bool hover = CheckCollisionPointRec(mouse, card);
            bool active = (i == unit);
            UI::GradientRound({ card.x, card.y + 4.0f, card.width, card.height }, 20.0f,
                              Theme::WithAlpha(Color{ 10, 17, 32, 255 }, 0.8f), Theme::WithAlpha(Color{ 4, 8, 18, 255 }, 0.8f));
            UI::GradientRound(card, 20.0f,
                              active ? Color{ 44, 68, 110, 255 } : (hover ? Color{ 34, 52, 86, 255 } : Color{ 25, 39, 68, 255 }),
                              Color{ 13, 22, 42, 255 });
            DrawRectangleRoundedLines(card, UI::Roundness(card, 20.0f), 16, active ? 3.0f : 1.5f,
                                      active ? Theme::gold : Theme::WithAlpha(Color{ 96, 165, 250, 255 }, hover ? 0.8f : 0.35f));
            // icon medallion
            Vector2 ic{ card.x + 64.0f, card.y + card.height * 0.5f };
            DrawCircleV({ ic.x, ic.y + 3.0f }, 50.0f, Theme::WithAlpha(BLACK, 0.3f));
            DrawCircleV(ic, 50.0f, Theme::WithAlpha(Color{ 30, 64, 175, 255 }, 0.35f));
            DrawCircleLines((int)ic.x, (int)ic.y, 50.0f, Theme::WithAlpha(WHITE, 0.2f));
            if (i == 0) Art::ScalesOfJustice({ ic.x, ic.y + 38.0f }, 0.52f);
            else if (i == 1) {
                DrawCircleV({ ic.x - 5.0f, ic.y + 4.0f }, 24.0f, Color{ 239, 68, 68, 255 });
                DrawCircleV({ ic.x - 11.0f, ic.y - 2.0f }, 5.0f, Theme::WithAlpha(WHITE, 0.35f));
                DrawEllipse((int)(ic.x + 18.0f), (int)(ic.y - 8.0f), 14.0f, 7.0f, Color{ 74, 222, 128, 255 });
                DrawLineEx({ ic.x + 18.0f, ic.y - 2.0f }, { ic.x + 22.0f, ic.y + 16.0f }, 2.0f, Color{ 21, 128, 61, 255 });
            } else if (i == 2) {
                DrawCircleV({ ic.x, ic.y + 2.0f }, 26.0f, Color{ 251, 191, 36, 255 });
                DrawRing(ic, 30.0f, 38.0f, 0.2f, 3.0f, 28, Color{ 244, 114, 182, 255 });
                DrawEllipse((int)(ic.x - 8.0f), (int)(ic.y - 8.0f), 8.0f, 5.0f, Theme::WithAlpha(WHITE, 0.35f));
            } else {
                Art::Heart({ ic.x, ic.y }, 34.0f, Color{ 244, 63, 94, 255 });
            }
            Theme::text3D(Theme::fontCartoon, names[i], { card.x + 138.0f, card.y + 34.0f }, 22.0f,
                          active ? Theme::goldLight : WHITE, Color{ 22, 30, 50, 255 }, 1.8f,
                          Theme::WithAlpha(BLACK, 0.45f));
            Theme::text(subs[i], { card.x + 140.0f, card.y + 72.0f }, 14.0f, Theme::WithAlpha(WHITE, 0.75f));
            int stars = starsInUnit(i);
            Theme::text3D(Theme::fontCartoon, std::to_string(stars) + " / " + std::to_string(gGameData.unitLevelCount(i) * 3),
                          { card.x + 140.0f, card.y + 104.0f }, 14.0f, Theme::goldLight,
                          Color{ 76, 40, 20, 255 }, 1.2f, Theme::WithAlpha(BLACK, 0.5f));
            for (int s = 0; s < 3; ++s) {
                UI::Star5({ card.x + 280.0f + s * 26.0f, card.y + 112.0f }, 10.0f, 4.5f,
                          (s < std::min(3, stars)) ? Theme::gold : Theme::WithAlpha(WHITE, 0.18f),
                          Theme::WithAlpha(Theme::goldDark, 0.7f), false);
            }
        }

        Rectangle close{ 448.0f, 88.0f, 38.0f, 38.0f };
        bool hov = CheckCollisionPointRec(mouse, close);
        UI::IconButton3D(close, Color{ 248, 113, 113, 255 }, Color{ 185, 28, 28, 255 }, Color{ 254, 202, 202, 255 }, hov);
        DrawLineEx({ close.x + 11.0f, close.y + 11.0f }, { close.x + 27.0f, close.y + 27.0f }, 3.0f, WHITE);
        DrawLineEx({ close.x + 27.0f, close.y + 11.0f }, { close.x + 11.0f, close.y + 27.0f }, 3.0f, WHITE);
    }

    void drawSettings() {
        DrawRectangle(0, 0, 540, 960, Theme::WithAlpha(BLACK, 0.62f));
        Rectangle panel{ 80.0f, 230.0f, 380.0f, 420.0f };
        UI::DropShadow(panel, 26.0f, 8.0f, 0.45f, 3.0f);
        UI::GradientRound(panel, 26.0f, Color{ 26, 39, 68, 252 }, Color{ 10, 17, 32, 252 });
        DrawRectangleRoundedLines(panel, UI::Roundness(panel, 26.0f), 16, 2.5f, Theme::WithAlpha(Theme::gold, 0.9f));
        Theme::text3D(Theme::fontCartoon, "SETTINGS", { 270.0f, 270.0f }, 30.0f,
                      Theme::goldLight, Color{ 76, 40, 20, 255 }, 2.0f, Theme::WithAlpha(BLACK, 0.55f), true, true);

        Vector2 mouse = Input::Mouse();
        Rectangle sound{ 112.0f, 330.0f, 316.0f, 58.0f };
        bool sh = CheckCollisionPointRec(mouse, sound);
        UI::Button3D(sound, "", Theme::WithAlpha(Color{ 30, 41, 59, 255 }, 0.9f),
                     Theme::WithAlpha(Color{ 15, 23, 42, 255 }, 0.95f), Color{ 8, 15, 30, 255 },
                     Color{ 71, 85, 105, 255 }, 18.0f, WHITE, sh);
        UI::SoundIcon({ sound.x + 42.0f, sound.y + 29.0f }, 15.0f, Theme::soundOn, WHITE);
        Theme::text(Theme::soundOn ? "SOUND: ON" : "SOUND: OFF", { sound.x + 74.0f, sound.y + 19.0f }, 18.0f, WHITE);

        Rectangle reset{ 112.0f, 406.0f, 316.0f, 58.0f };
        bool rh = CheckCollisionPointRec(mouse, reset);
        UI::Button3D(reset, "RESET PROGRESS", Color{ 248, 113, 113, 255 }, Color{ 220, 38, 38, 255 },
                     Color{ 127, 29, 29, 255 }, Color{ 254, 202, 202, 255 }, 18.0f, WHITE, rh);

        Rectangle close{ 112.0f, 500.0f, 316.0f, 58.0f };
        bool ch = CheckCollisionPointRec(mouse, close);
        UI::Button3D(close, "BACK TO MAP", Color{ 74, 222, 128, 255 }, Color{ 22, 163, 74, 255 },
                     Color{ 20, 83, 45, 255 }, Color{ 187, 247, 208, 255 }, 18.0f, WHITE, ch);
    }

    void handleOverlays(int& nextScreen) {
        Vector2 mouse = Input::Mouse();
        bool clicked = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
        if (selectorOpen) {
            Rectangle close{ 448.0f, 88.0f, 38.0f, 38.0f };
            if (clicked && CheckCollisionPointRec(mouse, close)) {
                selectorOpen = false;
                gAudio.playClick();
                return;
            }
            for (int i = 0; i < 4; ++i) {
                Rectangle card{ 52.0f, 168.0f + i * 166.0f, 436.0f, 142.0f };
                if (clicked && CheckCollisionPointRec(mouse, card)) {
                    selectorOpen = false;
                    gAudio.playWhoosh();
                    setUnit(i);
                    return;
                }
            }
            if (clicked && !CheckCollisionPointRec(mouse, { 28, 84, 484, 792 })) selectorOpen = false;
            return;
        }
        if (settingsOpen) {
            Rectangle sound{ 112.0f, 330.0f, 316.0f, 58.0f };
            Rectangle reset{ 112.0f, 406.0f, 316.0f, 58.0f };
            Rectangle close{ 112.0f, 500.0f, 316.0f, 58.0f };
            if (clicked && CheckCollisionPointRec(mouse, sound)) {
                Theme::soundOn = !Theme::soundOn;
                gAudio.enabled = Theme::soundOn;
                gAudio.playClick();
            } else if (clicked && CheckCollisionPointRec(mouse, reset)) {
                gGameData.resetProgress();
                setUnit(unit);
                gAudio.playClick();
                showToast("PROGRESS RESET");
            } else if (clicked && CheckCollisionPointRec(mouse, close)) {
                settingsOpen = false;
                gAudio.playClick();
            }
            (void)nextScreen;
            return;
        }
    }

    void handleInput(int screenW, int screenH, int& nextScreen) {
        (void)screenW;
        (void)screenH;
        handleOverlays(nextScreen);
        if (selectorOpen || settingsOpen) return;

        Vector2 mouse = Input::Mouse();
        bool clicked = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
        if (!clicked) return;

        // top chip chevrons / center
        if (unit != 2) {
            Rectangle chip{ 132.0f, 12.0f, 276.0f, 38.0f };
            if (CheckCollisionPointRec(mouse, chip)) {
                if (mouse.x < chip.x + 34.0f) {
                    setUnit((unit + 3) % 4);
                    gAudio.playWhoosh();
                } else if (mouse.x > chip.x + chip.width - 34.0f) {
                    setUnit((unit + 1) % 4);
                    gAudio.playWhoosh();
                } else {
                    selectorOpen = true;
                    gAudio.playClick();
                }
                return;
            }
        }

        // bottom buttons
        Rectangle gear{ 15.0f, unit == 2 ? 872.0f : 894.0f, 52.0f, 52.0f };
        Rectangle mapBtn{ 473.0f, unit == 2 ? 872.0f : 894.0f, 52.0f, 52.0f };
        if (CheckCollisionPointRec(mouse, gear)) {
            settingsOpen = true;
            gAudio.playClick();
            return;
        }
        if (CheckCollisionPointRec(mouse, mapBtn)) {
            selectorOpen = true;
            gAudio.playClick();
            return;
        }
        if (unit == 2) {
            Rectangle menu{ 476.0f, 13.0f, 48.0f, 48.0f };
            if (CheckCollisionPointRec(mouse, menu)) {
                settingsOpen = true;
                gAudio.playClick();
                return;
            }
        }

        // stage nodes
        int start = gGameData.unitStart(unit);
        for (const auto& n : unitNodes[unit].nodes) {
            if (!pointInNode(mouse, n)) continue;
            int gi = start + n.number - 1;
            if (gi < 0 || gi >= gGameData.levelCount()) return;
            auto& lvl = gGameData.levels[gi];
            bool unlocked = lvl.unlocked || verifyAllUnlocked;
            if (!unlocked) {
                gAudio.playClick();
                showToast("FINISH THE PREVIOUS STAGE FIRST");
                return;
            }
            gGameData.current_level = gi;
            nextScreen = Nav::stageScreenForLevel(gi);
            gAudio.playWhoosh();
            return;
        }
    }

    void draw(int screenW, int screenH) {
        if (unit <= 2 && cleanBg[unit].id > 0) {
            DrawTexturePro(cleanBg[unit], { 0, 0, (float)cleanBg[unit].width, (float)cleanBg[unit].height },
                           { 0, 0, (float)screenW, (float)screenH }, { 0, 0 }, 0.0f, WHITE);
        } else {
            Art::CarnivalIsland(screenW, screenH, time);
        }

        if (unit != 2) drawTopChip();
        else drawSpaceHUD();

        drawSigns();
        if (unit == 1) drawSafetyFoliage();
        drawNodes();
        if (unit != 2) drawBottomButtons();

        Theme::drawParticles();
        drawToast();

        if (selectorOpen) drawSelector();
        if (settingsOpen) drawSettings();
    }
};
