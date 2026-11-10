#pragma once
#include "raylib.h"
#include "input.hpp"
#include "audio_manager.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "theme.hpp"
#include "ui_widgets.hpp"
#include "victory_modal.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

class ScreenImageGuess {
public:
    VictoryModal victoryModal;
    Texture2D clueTexture{};
    std::string targetWord = "ECLIPSE";
    std::string currentGuess;
    std::vector<char> keys;
    std::vector<bool> keyUsed;
    int currentLevelIdx = 0;
    int hintsRemaining = 3;
    bool completed = false;
    bool wrongFlash = false;
    float wrongTimer = 0.0f;
    float time = 0.0f;
    float shake = 0.0f;
    float hintToastTimer = 0.0f;
    std::string hintToastText;

    void init() {
        victoryModal.init();
        loadLevel(gGameData.current_level);
    }

    void unload() {
        if (clueTexture.id > 0) UnloadTexture(clueTexture);
    }

    std::string resolveClueImage(const WordData& w) {
        if (w.image_keyword == "eclipse") return "assets/images/eclipse.png";
        if (w.image_keyword.find("counterfeit") != std::string::npos ||
            w.image_keyword.find("fake") != std::string::npos) return "assets/images/counterfeit.png";
        if (w.image_keyword.find("organic") != std::string::npos) return "assets/images/organic.png";
        if (w.image_keyword.find("galaxy") != std::string::npos ||
            w.image_keyword.find("nebula") != std::string::npos) return "assets/images/galaxy.png";
        if (w.image_keyword.find("telescope") != std::string::npos) return "assets/images/telescope.png";
        if (w.image_keyword.find("orbit") != std::string::npos ||
            w.image_keyword.find("planet") != std::string::npos ||
            w.image_keyword.find("asteroid") != std::string::npos) return "assets/images/planet.png";
        if (w.image_keyword.find("burger") != std::string::npos ||
            w.image_keyword.find("obesity") != std::string::npos) return "assets/images/junk_food.png";
        if (w.image_keyword.find("laugh") != std::string::npos) return "assets/images/laughter.png";
        if (w.image_keyword.find("stress") != std::string::npos) return "assets/images/stress.png";
        if (w.word == "ECLIPSE") return "assets/images/eclipse.png";
        if (w.word == "ORGANIC") return "assets/images/organic.png";
        if (w.word == "GALAXY") return "assets/images/galaxy.png";
        if (w.word == "TELESCOPE") return "assets/images/telescope.png";
        if (w.word == "COUNTERFEIT" || w.word == "FAKE") return "assets/images/counterfeit.png";
        if (w.word == "LAUGHTER") return "assets/images/laughter.png";
        return "assets/images/eclipse.png";
    }

    void loadLevel(int levelIdx) {
        currentLevelIdx = std::clamp(levelIdx, 0, std::max(0, gGameData.levelCount() - 1));
        if (gGameData.levelCount() == 0) return;
        auto& level = gGameData.levels[currentLevelIdx];
        targetWord = level.data.word;
        currentGuess.clear();
        completed = false;
        wrongFlash = false;
        wrongTimer = 0.0f;
        hintsRemaining = 3;
        hintToastTimer = 0.0f;
        victoryModal.hide();
        if (clueTexture.id > 0) {
            UnloadTexture(clueTexture);
            clueTexture = Texture2D{};
        }
        std::string path = resolveClueImage(level.data);
        if (FileExists(path.c_str())) clueTexture = Art::LoadRoundedTexture(path, 0.10f);
        setupKeyboard();
    }

    void resetStage() { loadLevel(gGameData.current_level); }

    void setupKeyboard() {
        keys.clear();
        keyUsed.clear();
        if (targetWord == "ECLIPSE") {
            std::string ks = "EPSOITLCRAENGY";
            for (char c : ks) keys.push_back(c);
        } else {
            for (char c : targetWord) keys.push_back(c);
            const std::string decoys = "ETAOINSRHLDCUM";
            for (char c : decoys) {
                if ((int)keys.size() >= 14) break;
                keys.push_back(c);
            }
            while ((int)keys.size() < 14) keys.push_back('Y');
            unsigned seed = (unsigned)currentLevelIdx * 2246822519u + 7u;
            for (size_t i = keys.size(); i > 1; --i) {
                seed = seed * 1664525u + 1013904223u;
                size_t j = seed % i;
                std::swap(keys[i - 1], keys[j]);
            }
        }
        keyUsed.assign(keys.size(), false);
    }

    void shuffleKeys() {
        unsigned seed = (unsigned)(GetTime() * 1000.0) + 13u;
        for (size_t i = keys.size(); i > 1; --i) {
            seed = seed * 1664525u + 1013904223u;
            size_t j = seed % i;
            std::swap(keys[i - 1], keys[j]);
            std::swap(keyUsed[i - 1], keyUsed[j]);
        }
    }

    void checkComplete() {
        if ((int)currentGuess.size() != (int)targetWord.size()) return;
        if (currentGuess == targetWord) {
            gAudio.playFanfare();
            complete();
        } else {
            wrongFlash = true;
            wrongTimer = 0.55f;
            shake = 10.0f;
            gAudio.playClick();
        }
    }

    void complete() {
        if (completed) return;
        completed = true;
        gGameData.completeLevel(currentLevelIdx, 3);
        victoryModal.show(gGameData.levels[currentLevelIdx].data);
    }

    void hint() {
        if (completed) return;
        if ((int)currentGuess.size() >= (int)targetWord.size()) return;
        // If the player has a wrong prefix, the hint restarts from the first
        // letter instead of appending a correct letter after a wrong one.
        if (targetWord.rfind(currentGuess, 0) != 0) clearWrong();
        char next = targetWord[currentGuess.size()];
        currentGuess += next;
        for (size_t i = 0; i < keys.size(); ++i) {
            if (!keyUsed[i] && keys[i] == next) {
                keyUsed[i] = true;
                break;
            }
        }
        if (hintsRemaining > 0) hintsRemaining--;
        hintToastText = gGameData.levels[currentLevelIdx].data.translation_ar;
        hintToastTimer = 2.4f;
        gAudio.playChime();
        checkComplete();
    }

    void clearWrong() {
        currentGuess.clear();
        for (size_t i = 0; i < keyUsed.size(); ++i) keyUsed[i] = false;
        wrongFlash = false;
        shake = 0.0f;
    }

    void typeLetter(int keyIndex) {
        if (keyIndex < 0 || keyIndex >= (int)keys.size()) return;
        if (keyUsed[keyIndex]) return;
        if ((int)currentGuess.size() >= (int)targetWord.size()) return;
        currentGuess += keys[keyIndex];
        keyUsed[keyIndex] = true;
        gAudio.playPop();
        checkComplete();
    }

    void backspace() {
        if (currentGuess.empty()) return;
        char removed = currentGuess.back();
        currentGuess.pop_back();
        for (int i = (int)keys.size() - 1; i >= 0; --i) {
            if (keyUsed[i] && keys[i] == removed) {
                keyUsed[i] = false;
                break;
            }
        }
    }

    void handleInput(int screenW, int screenH, int& nextScreen) {
        (void)screenW;
        (void)screenH;
        if (victoryModal.active) {
            bool next = false, map = false;
            victoryModal.handleInput(screenW, screenH, next, map);
            if (next) {
                nextScreen = Nav::stageScreenForLevel(gGameData.current_level);
                loadLevel(gGameData.current_level);
            } else if (map) {
                nextScreen = Nav::SCREEN_MAP;
            }
            return;
        }
        if (wrongFlash) return;
        Vector2 mouse = Input::Mouse();
        bool released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

        Rectangle pause{ 18.0f, 22.0f, 54.0f, 54.0f };
        if (released && CheckCollisionPointCircle(mouse, { 45.0f, 49.0f }, 28.0f)) {
            gAudio.playClick();
            nextScreen = Nav::SCREEN_MAP;
            return;
        }
        (void)pause;
        Rectangle bubble{ 318.0f, 105.0f, 106.0f, 62.0f };
        if (released && CheckCollisionPointRec(mouse, bubble)) {
            hint();
            return;
        }
        if (released && CheckCollisionPointRec(mouse, { 372.0f, 885.0f, 145.0f, 56.0f })) {
            gAudio.playFanfare();
            currentGuess = targetWord;
            complete();
            return;
        }
        if (released && CheckCollisionPointRec(mouse, { 24.0f, 884.0f, 94.0f, 62.0f })) {
            shuffleKeys();
            gAudio.playClick();
            return;
        }
        if (released && CheckCollisionPointRec(mouse, { 286.0f, 884.0f, 52.0f, 52.0f })) {
            backspace();
            gAudio.playClick();
            return;
        }

        // backspace via slots
        if (released) {
            float slotW, gap = 8.0f, total, startX;
            computeSlots(slotW, gap, total, startX);
            for (size_t i = 0; i < currentGuess.size(); ++i) {
                Rectangle s{ startX + (float)i * (slotW + gap), 618.0f, slotW, 64.0f };
                if (CheckCollisionPointRec(mouse, s)) {
                    // Remove the clicked letter and everything after it.
                    while ((int)currentGuess.size() > (int)i) backspace();
                    return;
                }
            }
        }

        // keyboard
        if (released) {
            for (int i = 0; i < (int)keys.size(); ++i) {
                Rectangle k = keyRect(i);
                if (CheckCollisionPointRec(mouse, k)) {
                    typeLetter(i);
                    return;
                }
            }
        }
        int k = GetKeyPressed();
        while (k > 0) {
            if (k >= 'A' && k <= 'Z') {
                for (int i = 0; i < (int)keys.size(); ++i) {
                    if (!keyUsed[i] && keys[i] == (char)k) {
                        typeLetter(i);
                        break;
                    }
                }
            } else if (k == KEY_BACKSPACE) {
                backspace();
            }
            k = GetKeyPressed();
        }
    }

    void computeSlots(float& slotW, float& gap, float& total, float& startX) const {
        int n = (int)targetWord.size();
        gap = 8.0f;
        slotW = std::min(62.0f, (472.0f - gap * (n - 1)) / std::max(1, n));
        total = n * slotW + (n - 1) * gap;
        startX = (540.0f - total) * 0.5f;
    }

    Rectangle keyRect(int i) const {
        int row = i / 7;
        int col = i % 7;
        return { 29.0f + col * 70.0f, 728.0f + row * 74.0f, 62.0f, 64.0f };
    }

    void drawPause() {
        Vector2 c{ 45.0f, 49.0f };
        DrawCircleV({ c.x, c.y + 3.0f }, 28.0f, Theme::WithAlpha(BLACK, 0.35f));
        DrawCircleGradient((int)c.x, (int)c.y, 28.0f, Color{ 96, 165, 250, 255 }, Color{ 37, 99, 235, 255 });
        DrawCircleLines((int)c.x, (int)c.y, 28.0f, Color{ 147, 197, 253, 255 });
        DrawCircleSector({ c.x, c.y - 2.0f }, 23.0f, 200.0f, 340.0f, 16, Theme::WithAlpha(WHITE, 0.22f));
        UI::PauseIcon(c, 13.0f, WHITE);
    }

    void drawRobotAndBubble() {
        Art::RobotMascot({ 482.0f, 218.0f }, 0.98f, time);
        Rectangle b{ 318.0f, 105.0f, 106.0f, 62.0f };
        Vector2 mouse = Input::Mouse();
        bool hover = CheckCollisionPointRec(mouse, b);
        UI::DropShadow(b, 16.0f, 4.0f, 0.30f, 1.0f);
        UI::GradientRound(b, 16.0f, WHITE, Color{ 226, 232, 240, 255 });
        DrawRectangleRoundedLines(b, UI::Roundness(b, 16.0f), 12, 2.0f, Color{ 203, 213, 225, 255 });
        // tail
        DrawTriangle({ b.x + b.width - 1.0f, b.y + 28.0f }, { b.x + b.width + 18.0f, b.y + 36.0f },
                     { b.x + b.width - 1.0f, b.y + 44.0f }, Color{ 241, 245, 249, 255 });
        Theme::text3D(Theme::fontCartoon, "NEED A", { b.x + b.width * 0.5f, b.y + 19.0f }, 14.0f,
                      Theme::ink, Color{ 148, 163, 184, 255 }, 0.8f, Theme::WithAlpha(BLACK, 0.25f), true, true);
        Theme::text3D(Theme::fontCartoon, "HINT?", { b.x + b.width * 0.5f, b.y + 40.0f }, 15.0f,
                      Theme::ink, Color{ 148, 163, 184, 255 }, 0.8f, Theme::WithAlpha(BLACK, 0.25f), true, true);
        // hint counter badge
        DrawCircleV({ b.x + b.width - 3.0f, b.y - 3.0f }, 13.0f, Color{ 239, 68, 68, 255 });
        DrawCircleLines((int)(b.x + b.width - 3.0f), (int)(b.y - 3.0f), 13.0f, Theme::WithAlpha(WHITE, 0.8f));
        Theme::text3D(Theme::fontCartoon, "x" + std::to_string(hintsRemaining),
                      { b.x + b.width - 3.0f, b.y - 2.0f }, 12.0f, WHITE,
                      Color{ 127, 29, 29, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.5f), true, true);
        if (hover) {
            DrawRectangleRoundedLines({ b.x - 2.0f, b.y - 2.0f, b.width + 4.0f, b.height + 4.0f },
                                      UI::Roundness(b, 18.0f), 12, 2.0f, Theme::WithAlpha(Theme::gold, 0.8f));
        }
        if (hintToastTimer > 0.0f) {
            float a = std::min(1.0f, hintToastTimer / 0.35f);
            Rectangle t{ 276.0f, 174.0f, 250.0f, 52.0f };
            DrawRectangleRounded(t, 0.35f, 12, Theme::WithAlpha(Color{ 15, 23, 42, 242 }, a));
            DrawRectangleRoundedLines(t, 0.35f, 12, 1.6f, Theme::WithAlpha(Theme::gold, a * 0.8f));
            UI::TextArabic(hintToastText, { t.x + t.width * 0.5f, t.y + t.height * 0.5f }, 20.0f,
                           Theme::WithAlpha(Theme::goldLight, a), true, true);
        }
    }

    void drawSlots() {
        float slotW, gap, total, startX;
        computeSlots(slotW, gap, total, startX);
        for (size_t i = 0; i < targetWord.size(); ++i) {
            Rectangle s{ startX + (float)i * (slotW + gap), 618.0f, slotW, 64.0f };
            std::string letter = (i < currentGuess.size()) ? std::string(1, currentGuess[i]) : "";
            bool cursor = (i == currentGuess.size());
            bool underscore = (i < 2 && i >= currentGuess.size());
            UI::drawDashedRoundedSlot(s, letter, cursor, underscore);
        }
    }

    void drawKeyboard() {
        // bottom console
        DrawRectangleGradientV(0, 710, 540, 250, Theme::WithAlpha(Color{ 19, 34, 64, 255 }, 0.96f),
                               Color{ 12, 21, 44, 255 });
        DrawLineEx({ 0, 711 }, { 540, 711 }, 2.0f, Theme::WithAlpha(Color{ 96, 165, 250, 255 }, 0.35f));
        DrawLineEx({ 0, 713 }, { 540, 713 }, 1.0f, Theme::WithAlpha(BLACK, 0.4f));
        Theme::text3D(Theme::fontCartoon, "TAP THE KEYS TO SPELL",
                      { 270.0f, 720.0f }, 11.0f, Theme::goldLight,
                      Color{ 76, 40, 20, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.5f), true, true);
        Vector2 mouse = Input::Mouse();
        for (int i = 0; i < (int)keys.size(); ++i) {
            Rectangle r = keyRect(i);
            bool used = keyUsed[i];
            bool hover = !used && CheckCollisionPointRec(mouse, r);
            if (used) {
                // recessed, disabled key
                DrawRectangleRounded({ r.x, r.y + 2.0f, r.width, r.height }, 0.22f, 8, Theme::WithAlpha(BLACK, 0.35f));
                DrawRectangleRounded(r, 0.22f, 8, Color{ 30, 41, 59, 255 });
                DrawRectangleRoundedLines(r, 0.22f, 8, 1.2f, Theme::WithAlpha(Color{ 71, 85, 105, 255 }, 0.7f));
                DrawRectangleRounded({ r.x + 4.0f, r.y + 3.0f, r.width - 8.0f, r.height * 0.25f }, 0.4f, 6,
                                     Theme::WithAlpha(WHITE, 0.04f));
            } else {
                UI::drawCreamKeyboardTile(r, std::string(1, keys[i]), hover, false);
            }
        }

        // shuffle
        Vector2 sc{ 70.0f, 902.0f };
        UI::ShuffleIcon(sc, 18.0f, Theme::goldLight);
        Theme::text3D(Theme::fontCartoon, "SHUFFLE", { sc.x, sc.y + 32.0f }, 13.0f,
                      Theme::goldLight, Color{ 120, 60, 25, 255 }, 1.2f, Theme::WithAlpha(BLACK, 0.6f), true, true);

        // delete / backspace
        Rectangle del{ 286.0f, 884.0f, 52.0f, 52.0f };
        bool delHover = CheckCollisionPointRec(mouse, del);
        UI::IconButton3D(del, Color{ 248, 113, 113, 255 }, Color{ 185, 28, 28, 255 },
                         Color{ 254, 202, 202, 255 }, delHover);
        Vector2 dc{ del.x + 26.0f, del.y + 26.0f };
        DrawLineEx({ dc.x - 12.0f, dc.y }, { dc.x + 13.0f, dc.y }, 3.2f, WHITE);
        DrawTriangle({ dc.x - 16.0f, dc.y }, { dc.x - 6.0f, dc.y - 7.0f }, { dc.x - 6.0f, dc.y + 7.0f }, WHITE);
        DrawLineEx({ dc.x + 6.0f, dc.y - 8.0f }, { dc.x + 12.0f, dc.y + 8.0f }, 2.4f, WHITE);
        DrawLineEx({ dc.x + 12.0f, dc.y - 8.0f }, { dc.x + 6.0f, dc.y + 8.0f }, 2.4f, WHITE);

        // skip
        Rectangle skip{ 372.0f, 885.0f, 145.0f, 56.0f };
        bool sh = CheckCollisionPointRec(mouse, skip);
        UI::DropShadow(skip, 24.0f, 5.0f, 0.4f, 1.0f);
        UI::GradientRound({ skip.x, skip.y + 4.0f, skip.width, skip.height }, 24.0f,
                          Color{ 180, 30, 30, 255 }, Color{ 127, 29, 29, 255 });
        UI::GradientRound(skip, 24.0f, sh ? Color{ 251, 146, 60, 255 } : Color{ 249, 115, 22, 255 },
                          Color{ 239, 68, 68, 255 });
        DrawRectangleRoundedLines(skip, UI::Roundness(skip, 24.0f), 16, 2.0f, Theme::goldLight);
        DrawRectangleRounded({ skip.x + 8.0f, skip.y + 4.0f, skip.width - 16.0f, 18.0f }, 0.7f, 8,
                             Theme::WithAlpha(WHITE, 0.18f));
        Theme::text3D(Theme::fontCartoon, "SKIP  >>|", { skip.x + skip.width * 0.5f, skip.y + 28.0f }, 21.0f,
                      WHITE, Color{ 153, 27, 27, 255 }, 1.8f, Theme::WithAlpha(BLACK, 0.45f), true, true);
    }

    void draw(int screenW, int screenH) {
        float sx = 0.0f;
        if (wrongFlash) sx = std::sin(time * 55.0f) * shake;
        Art::AcademicPattern(screenW, screenH);
        drawPause();
        UI::drawPencilProgressBar({ 128.0f, 27.0f, 268.0f, 38.0f }, currentLevelIdx + 1,
                                  std::max(1, gGameData.levelCount()));
        UI::CoinsBadge({ 422.0f, 24.0f }, 350, 102.0f, 40.0f);

        BeginScissorMode((int)sx, 0, screenW, screenH);
        drawRobotAndBubble();
        Art::AcademicFrame({ 82.0f, 178.0f, 376.0f, 392.0f }, clueTexture);
        drawSlots();
        EndScissorMode();

        drawKeyboard();
        Theme::drawParticles();

        if (victoryModal.active) {
            bool n = false, m = false;
            victoryModal.draw(gGameData.levels[currentLevelIdx], screenW, screenH, n, m);
        }
    }

    void update(float dt) {
        time += dt;
        if (hintToastTimer > 0.0f) hintToastTimer -= dt;
        if (wrongFlash) {
            wrongTimer -= dt;
            shake *= 0.90f;
            if (wrongTimer <= 0.0f) clearWrong();
        }
        if (victoryModal.active) victoryModal.update(dt);
    }
};
