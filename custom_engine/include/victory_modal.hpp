#pragma once
#include "raylib.h"
#include "input.hpp"
#include "audio_manager.hpp"
#include "game_data.hpp"
#include "theme.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <string>

class VictoryModal {
public:
    bool active = false;
    float anim = 0.0f;
    bool nextClicked = false;
    bool mapClicked = false;

    void init() {}
    void show(const WordData& word) {
        (void)word;
        active = true;
        anim = 0.0f;
        Theme::spawnCelebration({ 270.0f, 420.0f }, 64);
    }
    void hide() { active = false; }

    void update(float dt) {
        if (active) anim += dt;
    }

    void handleInput(int screenW, int screenH, bool& next_clicked, bool& map_clicked) {
        next_clicked = false;
        map_clicked = false;
        if (!active) return;
        float card_w = 486.0f;
        float card_h = 602.0f;
        float card_x = (screenW - card_w) * 0.5f;
        float card_y = (screenH - card_h) * 0.5f;
        Rectangle mapBtn{ card_x + 26.0f, card_y + card_h - 78.0f, 202.0f, 54.0f };
        Rectangle nextBtn{ card_x + card_w - 228.0f, card_y + card_h - 78.0f, 202.0f, 54.0f };
        if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;
        Vector2 mouse = Input::Mouse();
        if (CheckCollisionPointRec(mouse, mapBtn)) {
            gAudio.playClick();
            map_clicked = true;
            active = false;
        } else if (CheckCollisionPointRec(mouse, nextBtn)) {
            gAudio.playClick();
            int nextLevel = gGameData.current_level + 1;
            if (nextLevel < gGameData.levelCount()) {
                next_clicked = true;
                gGameData.current_level = nextLevel;
            } else {
                // "FINISH" on the last level returns to the island map.
                map_clicked = true;
            }
            active = false;
        }
    }

    float fitArabic(const std::string& raw, float size, float maxWidth) {
        std::string shaped = Theme::shapeArabic(raw);
        float w = MeasureTextEx(Theme::fontArabic, shaped.c_str(), size, 0.0f).x;
        while (w > maxWidth && size > 18.0f) {
            size -= 1.0f;
            w = MeasureTextEx(Theme::fontArabic, shaped.c_str(), size, 0.0f).x;
        }
        return size;
    }

    void draw(const LevelInfo& lvl, int screenW, int screenH, bool& next_clicked, bool& map_clicked) {
        if (!active) return;
        next_clicked = false;
        map_clicked = false;

        float t = std::min(1.0f, anim / 0.28f);
        float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
        float card_w = 486.0f;
        float card_h = 602.0f;
        float card_x = (screenW - card_w) * 0.5f;
        float card_y = (screenH - card_h) * 0.5f + (1.0f - ease) * 26.0f;

        DrawRectangle(0, 0, screenW, screenH, Theme::WithAlpha(Color{ 4, 8, 20, 255 }, 0.72f * ease));

        // radial light rays behind the card
        Vector2 center{ (float)screenW * 0.5f, card_y + 130.0f };
        for (int i = 0; i < 12; ++i) {
            float a = (float)i * PI / 6.0f + anim * 0.08f;
            Vector2 p1{ center.x + std::cos(a - 0.05f) * 700.0f, center.y + std::sin(a - 0.05f) * 700.0f };
            Vector2 p2{ center.x + std::cos(a + 0.05f) * 700.0f, center.y + std::sin(a + 0.05f) * 700.0f };
            DrawTriangle(center, p1, p2, Theme::WithAlpha(Theme::gold, 0.035f * ease));
        }

        UI::DropShadow({ card_x, card_y, card_w, card_h }, 28.0f, 12.0f, 0.55f, 4.0f);
        // gold frame + dark glass body
        UI::GradientRound({ card_x, card_y + 7.0f, card_w, card_h }, 28.0f,
                          Color{ 180, 83, 9, 255 }, Color{ 120, 53, 15, 255 });
        UI::GradientRound({ card_x, card_y, card_w, card_h }, 28.0f,
                          Color{ 251, 191, 36, 255 }, Color{ 217, 119, 6, 255 });
        Rectangle inner{ card_x + 5.0f, card_y + 5.0f, card_w - 10.0f, card_h - 10.0f };
        UI::GradientRound(inner, 24.0f, Color{ 28, 42, 72, 255 }, Color{ 10, 17, 34, 255 });
        DrawRectangleRoundedLines(inner, UI::Roundness(inner, 24.0f), 16, 2.0f, Theme::WithAlpha(Theme::goldLight, 0.9f));
        DrawRectangleRoundedLines({ card_x - 1.0f, card_y - 1.0f, card_w + 2.0f, card_h + 2.0f },
                                  UI::Roundness({ 0, 0, card_w, card_h }, 28.0f), 16, 1.4f,
                                  Theme::WithAlpha(WHITE, 0.55f));

        // header ribbon
        Rectangle ribbon{ card_x + 26.0f, card_y + 18.0f, card_w - 52.0f, 46.0f };
        UI::GradientRound(ribbon, 16.0f, Color{ 250, 204, 21, 255 }, Color{ 234, 130, 20, 255 });
        DrawRectangleRoundedLines(ribbon, UI::Roundness(ribbon, 16.0f), 12, 1.8f, Color{ 146, 64, 14, 255 });
        Theme::text3D(Theme::fontCartoon, "STAGE COMPLETED!", { card_x + card_w * 0.5f, ribbon.y + 23.0f },
                      26.0f, WHITE, Color{ 146, 64, 14, 255 }, 2.2f, Theme::WithAlpha(BLACK, 0.5f), true, true);

        // stars pop in
        for (int s = -1; s <= 1; ++s) {
            float local = std::clamp((anim - 0.12f - (s + 1) * 0.08f) / 0.25f, 0.0f, 1.0f);
            float sc = 0.5f + 0.5f * local;
            float bounce = std::sin(local * PI) * 6.0f;
            Vector2 sp{ card_x + card_w * 0.5f + s * 66.0f, card_y + 104.0f - bounce };
            UI::Star5(sp, 30.0f * sc, 13.0f * sc, Theme::gold, Color{ 146, 64, 14, 255 }, true);
            UI::Sparkle({ sp.x - 12.0f, sp.y - 9.0f }, 6.0f * sc, Theme::WithAlpha(WHITE, 0.9f));
        }

        // solved word
        Theme::text3D(Theme::fontCartoon, lvl.data.word, { card_x + card_w * 0.5f, card_y + 168.0f },
                      40.0f, WHITE, Color{ 15, 23, 42, 255 }, 2.6f, Theme::WithAlpha(BLACK, 0.6f), true, true);

        // Arabic translation card (the pedagogical heart of the game)
        Rectangle ar{ card_x + 24.0f, card_y + 204.0f, card_w - 48.0f, 96.0f };
        UI::DropShadow(ar, 18.0f, 4.0f, 0.35f, 1.0f);
        UI::GradientRound({ ar.x, ar.y + 4.0f, ar.width, ar.height }, 18.0f, Color{ 20, 83, 45, 255 }, Color{ 6, 78, 59, 255 });
        UI::GradientRound(ar, 18.0f, Color{ 74, 222, 128, 255 }, Color{ 22, 163, 74, 255 });
        DrawRectangleRoundedLines(ar, UI::Roundness(ar, 18.0f), 14, 2.2f, Color{ 187, 247, 208, 255 });
        DrawRectangleRounded({ ar.x + 8.0f, ar.y + 5.0f, ar.width - 16.0f, 26.0f }, 0.6f, 8,
                             Theme::WithAlpha(WHITE, 0.12f));
        UI::TextArabic("الترجمة العربية", { ar.x + ar.width - 82.0f, ar.y + 19.0f }, 17.0f,
                       Color{ 220, 252, 231, 255 }, true, true);
        float arSize = fitArabic(lvl.data.translation_ar, 40.0f, ar.width - 40.0f);
        UI::TextArabic(lvl.data.translation_ar, { ar.x + ar.width * 0.5f, ar.y + 56.0f }, arSize,
                       Color{ 254, 249, 195, 255 }, true, true, Color{ 6, 78, 59, 255 }, 1.4f);
        DrawLineEx({ ar.x + 24.0f, ar.y + 82.0f }, { ar.x + ar.width - 24.0f, ar.y + 82.0f }, 1.0f,
                   Theme::WithAlpha(WHITE, 0.18f));

        // bac data card
        Rectangle bac{ card_x + 24.0f, card_y + 312.0f, card_w - 48.0f, 180.0f };
        UI::GlassPanel(bac, 16.0f, Color{ 22, 34, 58, 230 }, Theme::WithAlpha(Color{ 96, 165, 250, 255 }, 0.5f));
        Theme::text3D(Theme::fontCartoon, "BAC EXAM DEFINITION & SYNONYM",
                      { bac.x + 16.0f, bac.y + 14.0f }, 13.0f, Color{ 251, 191, 36, 255 },
                      Color{ 76, 40, 20, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.45f));

        auto defLines = UI::WrapText("Definition: " + lvl.data.definition_en, 14.0f, bac.width - 32.0f);
        float yy = bac.y + 38.0f;
        for (const auto& line : defLines) {
            Theme::text(line, { bac.x + 16.0f, yy }, 14.0f, Color{ 241, 245, 249, 255 });
            yy += 18.0f;
        }
        Theme::text("Synonyms: " + lvl.data.synonym_en, { bac.x + 16.0f, yy + 4.0f }, 14.0f,
                    Color{ 134, 239, 172, 255 });
        Theme::text(gGameData.macronFrequencyEn(lvl.data.frequency), { bac.x + 16.0f, yy + 26.0f }, 13.0f,
                    Theme::WithAlpha(WHITE, 0.8f));
        std::string freqAr = gGameData.macronFrequencyAr(lvl.data.frequency);
        float freqSize = fitArabic(freqAr, 19.0f, bac.width - 40.0f);
        UI::TextArabic(freqAr, { bac.x + bac.width * 0.5f, bac.y + bac.height - 23.0f }, freqSize,
                       Color{ 253, 224, 71, 255 }, true, true);

        // buttons
        Vector2 mouse = Input::Mouse();
        Rectangle mapBtn{ card_x + 26.0f, card_y + card_h - 78.0f, 202.0f, 54.0f };
        Rectangle nextBtn{ card_x + card_w - 228.0f, card_y + card_h - 78.0f, 202.0f, 54.0f };
        bool mh = CheckCollisionPointRec(mouse, mapBtn);
        bool nh = CheckCollisionPointRec(mouse, nextBtn);
        UI::Button3D(mapBtn, "ISLAND MAP", Color{ 96, 165, 250, 255 }, Color{ 37, 99, 235, 255 },
                     Color{ 30, 58, 138, 255 }, Color{ 191, 219, 254, 255 }, 18.0f, WHITE, mh);
        int nextLevel = lvl.global_index + 1;
        std::string nextLabel = (nextLevel < gGameData.levelCount()) ? "NEXT LEVEL" : "FINISH";
        UI::Button3D(nextBtn, nextLabel, Color{ 74, 222, 128, 255 }, Color{ 22, 163, 74, 255 },
                     Color{ 20, 83, 45, 255 }, Color{ 187, 247, 208, 255 }, 18.0f, WHITE, nh);
        // arrows
        DrawLineEx({ nextBtn.x + nextBtn.width - 34.0f, nextBtn.y + 22.0f },
                   { nextBtn.x + nextBtn.width - 24.0f, nextBtn.y + 27.0f }, 2.5f, WHITE);
        DrawLineEx({ nextBtn.x + nextBtn.width - 24.0f, nextBtn.y + 27.0f },
                   { nextBtn.x + nextBtn.width - 34.0f, nextBtn.y + 32.0f }, 2.5f, WHITE);

        (void)mh;
        (void)nh;
    }
};
