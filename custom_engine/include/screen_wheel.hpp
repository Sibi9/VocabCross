#pragma once
#include "raylib.h"
#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_widgets.hpp"
#include "victory_modal.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

class ScreenWheel {
public:
    struct Layout {
        float tile = 40.0f;
        float gap = 4.0f;
        float originX = 0.0f;
        float originY = 0.0f;
    };

    VictoryModal victoryModal;
    CrosswordPuzzle puzzle;
    std::string targetWord;
    std::vector<char> wheelLetters;
    std::vector<Color> wheelColors;
    std::vector<Vector2> letterPos;
    std::vector<int> selectedIndices;
    Layout layout;

    int currentLevelIdx = 0;
    int hintsRemaining = 3;
    bool completed = false;
    bool dragging = false;
    bool wrongFlash = false;
    float wrongTimer = 0.0f;
    float wrongShake = 0.0f;
    float time = 0.0f;
    float hintToastTimer = 0.0f;
    std::string hintToastText;

    static constexpr Vector2 wheelCenter{ 270.0f, 735.0f };
    static constexpr float wheelRadius = 176.0f;
    static constexpr float letterRadius = 118.0f;

    void init() {
        victoryModal.init();
        loadLevel(gGameData.current_level);
    }

    void unload() {}

    void loadLevel(int levelIdx) {
        currentLevelIdx = std::clamp(levelIdx, 0, std::max(0, gGameData.levelCount() - 1));
        if (gGameData.levelCount() == 0) return;
        auto& level = gGameData.levels[currentLevelIdx];
        targetWord = level.data.word;
        // Two real crossing words keep the wheel readable (10-16 letters).
        puzzle.build(level.data, gGameData.units[level.unit_index], 1);
        completed = false;
        dragging = false;
        wrongFlash = false;
        wrongTimer = 0.0f;
        hintsRemaining = 3;
        hintToastTimer = 0.0f;
        victoryModal.hide();
        selectedIndices.clear();
        buildWheel();
        computeLayout();
    }

    void resetStage() { loadLevel(gGameData.current_level); }

    void buildWheel() {
        wheelLetters = puzzle.supplyLetters();
        unsigned seed = (unsigned)currentLevelIdx * 2654435761u + 97u;
        for (size_t i = wheelLetters.size(); i > 1; --i) {
            seed = seed * 1664525u + 1013904223u;
            size_t j = seed % i;
            std::swap(wheelLetters[i - 1], wheelLetters[j]);
        }
        static const Color palette[] = {
            { 239, 68, 68, 255 }, { 74, 222, 128, 255 }, { 251, 191, 36, 255 },
            { 56, 189, 248, 255 }, { 192, 132, 252, 255 }, { 244, 114, 182, 255 },
            { 251, 146, 60, 255 }, { 45, 212, 191, 255 }, { 129, 140, 248, 255 },
            { 250, 204, 21, 255 }, { 134, 239, 172, 255 }, { 253, 164, 175, 255 }
        };
        wheelColors.clear();
        for (size_t i = 0; i < wheelLetters.size(); ++i) wheelColors.push_back(palette[i % 12]);
        letterPos.resize(wheelLetters.size());
        for (size_t i = 0; i < wheelLetters.size(); ++i) {
            float a = -PI * 0.5f + (float)i * 2.0f * PI / std::max<size_t>(1, wheelLetters.size());
            letterPos[i] = { wheelCenter.x + std::cos(a) * letterRadius,
                             wheelCenter.y + std::sin(a) * letterRadius };
        }
    }

    void computeLayout() {
        int cols = std::max(1, puzzle.cols());
        int rows = std::max(1, puzzle.rows());
        float gap = 4.0f;
        float tile = std::min(44.0f, (450.0f - gap * (cols - 1)) / cols);
        tile = std::min(tile, (295.0f - gap * (rows - 1)) / rows);
        tile = std::max(22.0f, tile);
        float totalW = cols * tile + (cols - 1) * gap;
        float totalH = rows * tile + (rows - 1) * gap;
        layout.tile = tile;
        layout.gap = gap;
        layout.originX = (540.0f - totalW) * 0.5f;
        layout.originY = 150.0f + (295.0f - totalH) * 0.5f;
    }

    Rectangle cellRect(const PuzzleCell& c) const {
        float step = layout.tile + layout.gap;
        return { layout.originX + c.col * step, layout.originY + c.row * step, layout.tile, layout.tile };
    }

    std::string currentWord() const {
        std::string s;
        for (int idx : selectedIndices) {
            if (idx >= 0 && idx < (int)wheelLetters.size()) s += wheelLetters[idx];
        }
        return s;
    }

    bool isPrefixOfUnsolved(const std::string& s) const {
        for (const auto& w : puzzle.words) {
            if (!w.solved && w.answer.rfind(s, 0) == 0) return true;
        }
        return false;
    }

    int matchingUnsolvedWord(const std::string& s) const {
        for (int i = 0; i < (int)puzzle.words.size(); ++i) {
            if (!puzzle.words[i].solved && puzzle.words[i].answer == s) return i;
        }
        return -1;
    }

    bool matchesSolvedWord(const std::string& s) const {
        for (const auto& w : puzzle.words) {
            if (w.solved && w.answer == s) return true;
        }
        return false;
    }

    // Called on every new letter.  It only reacts to a *complete* word; it must
    // never fail a partial path, otherwise a drag dies on its third letter.
    void evaluateSelection() {
        std::string word = currentWord();
        if (word.empty()) return;
        int mi = matchingUnsolvedWord(word);
        if (mi >= 0) {
            puzzle.solveWord(mi);
            hintToastText = puzzle.words[mi].data.translation_ar;
            hintToastTimer = 2.2f;
            selectedIndices.clear();
            dragging = false;
            if (puzzle.allSolved()) {
                gAudio.playFanfare();
                complete();
            } else {
                gAudio.playChime();
            }
        }
    }

    // Called only when the player releases the mouse.  A short prefix is kept
    // so tap-to-spell continues naturally; a completed but wrong path shakes.
    void finishSelection() {
        dragging = false;
        std::string word = currentWord();
        if (word.empty()) return;
        if (matchesSolvedWord(word)) {
            clearSelection();
            return;
        }
        int mi = matchingUnsolvedWord(word);
        if (mi >= 0) {
            evaluateSelection();
            return;
        }
        if ((int)word.size() >= 3 && !isPrefixOfUnsolved(word)) {
            wrongFlash = true;
            wrongTimer = 0.55f;
            wrongShake = 10.0f;
            gAudio.playClick();
        }
    }

    void selectLetter(int idx) {
        if (completed || idx < 0 || idx >= (int)wheelLetters.size()) return;
        if (!selectedIndices.empty() && selectedIndices.back() == idx) {
            selectedIndices.pop_back();
            gAudio.playPop();
            return;
        }
        if (std::find(selectedIndices.begin(), selectedIndices.end(), idx) != selectedIndices.end()) return;
        int maxLen = 0;
        for (const auto& w : puzzle.words) maxLen = std::max(maxLen, (int)w.answer.size());
        if ((int)selectedIndices.size() >= maxLen) return;
        selectedIndices.push_back(idx);
        gAudio.playPop();
        evaluateSelection();
    }

    void startDrag(Vector2 mouse) {
        int idx = hitLetter(mouse);
        if (idx < 0) return;
        dragging = true;
        selectLetter(idx);
    }

    void continueDrag(Vector2 mouse) {
        if (!dragging) return;
        int idx = hitLetter(mouse);
        if (idx < 0) return;
        if (std::find(selectedIndices.begin(), selectedIndices.end(), idx) != selectedIndices.end()) return;
        int maxLen = 0;
        for (const auto& w : puzzle.words) maxLen = std::max(maxLen, (int)w.answer.size());
        if ((int)selectedIndices.size() >= maxLen) return;
        selectedIndices.push_back(idx);
        gAudio.playPop();
        evaluateSelection();
    }

    void endDrag() { finishSelection(); }

    void clearSelection() {
        selectedIndices.clear();
        wrongFlash = false;
        wrongShake = 0.0f;
        dragging = false;
    }

    int hitLetter(Vector2 p) const {
        float hitR = wheelLetters.size() > 13 ? 19.0f : (wheelLetters.size() > 9 ? 22.0f : 28.0f);
        for (size_t i = 0; i < letterPos.size(); ++i) {
            if (CheckCollisionPointCircle(p, letterPos[i], hitR)) return (int)i;
        }
        return -1;
    }

    void complete() {
        if (completed) return;
        completed = true;
        gGameData.completeLevel(currentLevelIdx, 3);
        victoryModal.show(gGameData.levels[currentLevelIdx].data);
    }

    void shuffleWheel() {
        if (completed) return;
        unsigned seed = (unsigned)(GetTime() * 997.0) + 31u;
        for (size_t i = wheelLetters.size(); i > 1; --i) {
            seed = seed * 1664525u + 1013904223u;
            size_t j = seed % i;
            std::swap(wheelLetters[i - 1], wheelLetters[j]);
        }
        selectedIndices.clear();
        wrongFlash = false;
        wrongShake = 0.0f;
        gAudio.playClick();
    }

    int shortestUnsolved() const {
        int best = -1;
        for (int i = 0; i < (int)puzzle.words.size(); ++i) {
            if (puzzle.words[i].solved) continue;
            if (best < 0 || puzzle.words[i].answer.size() < puzzle.words[best].answer.size()) best = i;
        }
        return best;
    }

    void hint() {
        if (completed) return;
        int wi = shortestUnsolved();
        if (wi < 0) return;
        std::string prefix = currentWord();
        int targetWi = -1;
        if (!prefix.empty() && puzzle.words[wi].answer.rfind(prefix, 0) == 0) targetWi = wi;
        else {
            for (int i = 0; i < (int)puzzle.words.size(); ++i) {
                if (!puzzle.words[i].solved && !prefix.empty() &&
                    puzzle.words[i].answer.rfind(prefix, 0) == 0) { targetWi = i; break; }
            }
        }
        if (targetWi < 0) {
            selectedIndices.clear();
            targetWi = wi;
            prefix.clear();
        }
        if ((int)prefix.size() >= (int)puzzle.words[targetWi].answer.size()) return;
        char need = puzzle.words[targetWi].answer[prefix.size()];
        for (size_t i = 0; i < wheelLetters.size(); ++i) {
            if (std::find(selectedIndices.begin(), selectedIndices.end(), (int)i) != selectedIndices.end()) continue;
            if (wheelLetters[i] == need) {
                selectedIndices.push_back((int)i);
                break;
            }
        }
        if (hintsRemaining > 0) hintsRemaining--;
        hintToastText = puzzle.words[targetWi].data.translation_ar;
        hintToastTimer = 2.4f;
        gAudio.playChime();
        evaluateSelection();
    }

    void demoSubmitSelection() { finishSelection(); }

    void setDemoSelection(const std::string& prefix) {
        selectedIndices.clear();
        std::vector<bool> used(wheelLetters.size(), false);
        for (char c : prefix) {
            bool found = false;
            for (size_t i = 0; i < wheelLetters.size(); ++i) {
                if (!used[i] && wheelLetters[i] == c) {
                    used[i] = true;
                    selectedIndices.push_back((int)i);
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
        // Synthetic demo state: draw the selected path, but do not append the
        // OS cursor (which is at 0,0 in --verify-ui).
        dragging = false;
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
        Rectangle settings{ 462.0f, 16.0f, 52.0f, 52.0f };
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, settings)) {
            gAudio.playClick();
            nextScreen = Nav::SCREEN_MAP;
            return;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mouse, { 70.0f, 906.0f }, 36.0f)) {
            shuffleWheel();
            return;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(mouse, { 470.0f, 906.0f }, 36.0f)) {
            hint();
            return;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !selectedIndices.empty() &&
            CheckCollisionPointRec(mouse, { 214.0f, 526.0f, 112.0f, 28.0f })) {
            clearSelection();
            gAudio.playClick();
            return;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) startDrag(mouse);
        if (dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) continueDrag(mouse);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && dragging) endDrag();
    }

    void drawWhiteGrid() {
        std::vector<int> starts(puzzle.cells.size(), 0);
        for (int wi = 0; wi < (int)puzzle.words.size(); ++wi) {
            auto indices = puzzle.wordCellIndices(wi);
            if (!indices.empty()) starts[indices.front()] = wi + 1;
        }
        for (const auto& c : puzzle.cells) {
            Rectangle r = cellRect(c);
            bool solved = c.locked;
            if (!solved) {
                DrawRectangleRounded({ r.x - 4.0f, r.y - 4.0f, r.width + 8.0f, r.height + 8.0f },
                                     UI::Roundness(r, 10.0f), 12, Theme::WithAlpha(Theme::gold, 0.18f));
            }
            std::string letter = (c.entered == '_') ? "" : std::string(1, c.entered);
            UI::drawGlossyWhiteTile3D(r, letter, false);
            DrawRectangleRoundedLines({ r.x - 1.0f, r.y - 1.0f, r.width + 2.0f, r.height + 2.0f },
                                      UI::Roundness(r, 10.0f), 12, 2.0f,
                                      solved ? Color{ 74, 222, 128, 255 } : Theme::gold);
            if (letter.empty()) {
                DrawLineEx({ r.x + r.width * 0.28f, r.y + r.height - 8.0f },
                           { r.x + r.width * 0.72f, r.y + r.height - 8.0f }, 1.8f,
                           Theme::WithAlpha(Theme::goldLight, 0.8f));
            }
            int ci = puzzle.cellIndex(c.row, c.col);
            if (ci >= 0 && ci < (int)starts.size() && starts[ci] > 0) {
                Theme::text3D(Theme::fontCartoon, std::to_string(starts[ci]),
                              { r.x + 4.0f, r.y + 2.0f }, 8.0f, Theme::goldLight,
                              Color{ 76, 40, 20, 255 }, 0.7f, Theme::WithAlpha(BLACK, 0.5f));
            }
        }
    }

    Rectangle clearButtonRect() const { return { 214.0f, 526.0f, 112.0f, 28.0f }; }

    void drawPrompt() {
        Theme::text3D(Theme::fontCartoon, "FIND THE HIDDEN WORDS",
                      { 270.0f, 458.0f }, 16.0f, Theme::goldLight,
                      Color{ 76, 40, 20, 255 }, 1.8f, Theme::WithAlpha(BLACK, 0.55f), true, true);
        std::string info = std::to_string(puzzle.words.size()) + " WORDS";
        for (const auto& w : puzzle.words) info += "  •  " + std::to_string(w.answer.size());
        Theme::text(info, { 270.0f, 480.0f }, 11.0f, Theme::WithAlpha(WHITE, 0.70f), true);

        std::string preview;
        int maxLen = 0;
        for (const auto& w : puzzle.words) maxLen = std::max(maxLen, (int)w.answer.size());
        for (int i = 0; i < maxLen; ++i) {
            if (i > 0) preview += " ";
            if (i < (int)selectedIndices.size()) preview += currentWord()[i];
            else preview += "_";
        }
        Color previewCol = Theme::WithAlpha(WHITE, 0.45f);
        if (!selectedIndices.empty()) {
            previewCol = isPrefixOfUnsolved(currentWord()) ? WHITE : Color{ 255, 160, 170, 255 };
        }
        Theme::text3D(Theme::fontCartoon, preview, { 270.0f, 501.0f }, 14.0f,
                      previewCol, Color{ 22, 30, 50, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.45f), true, true);

        if (!selectedIndices.empty()) {
            Rectangle r = clearButtonRect();
            UI::Capsule(r, Color{ 30, 43, 68, 240 }, Color{ 12, 20, 38, 245 },
                        Theme::WithAlpha(Theme::rose, 0.7f));
            Theme::text3D(Theme::fontCartoon, "CLEAR", { r.x + r.width * 0.5f, r.y + r.height * 0.5f },
                          12.0f, WHITE, Color{ 127, 29, 29, 255 }, 1.0f,
                          Theme::WithAlpha(BLACK, 0.45f), true, true);
        }
    }

    void drawWheelBackground() {
        DrawRing({ wheelCenter.x, wheelCenter.y + 5.0f }, wheelRadius - 13.0f, wheelRadius + 2.0f, 0.0f, 360.0f, 120,
                 Color{ 180, 83, 9, 255 });
        DrawRing(wheelCenter, wheelRadius - 12.0f, wheelRadius + 1.0f, 0.0f, 360.0f, 120,
                 Color{ 217, 119, 6, 255 });
        DrawRing(wheelCenter, wheelRadius - 9.0f, wheelRadius - 1.0f, 0.0f, 360.0f, 120,
                 Color{ 253, 224, 71, 255 });
        DrawRing(wheelCenter, wheelRadius - 5.0f, wheelRadius - 2.0f, 0.0f, 360.0f, 120,
                 Theme::WithAlpha(WHITE, 0.75f));
        DrawRingLines(wheelCenter, wheelRadius - 13.0f, wheelRadius + 2.0f, 0.0f, 360.0f, 120,
                      Theme::WithAlpha(WHITE, 0.45f));
        DrawRing(wheelCenter, wheelRadius + 2.0f, wheelRadius + 5.0f, 0.0f, 360.0f, 120,
                 Theme::WithAlpha(Color{ 103, 232, 249, 255 }, 0.55f));
        DrawCircleV(wheelCenter, wheelRadius - 14.0f, Color{ 96, 137, 196, 242 });
        DrawCircleGradient((int)wheelCenter.x, (int)(wheelCenter.y - 20.0f), wheelRadius - 18.0f,
                           Color{ 198, 228, 255, 215 }, Color{ 108, 153, 214, 185 });
        DrawCircleV(wheelCenter, wheelRadius - 16.0f, Theme::WithAlpha(Color{ 146, 118, 218, 255 }, 0.10f));
        DrawCircleV(wheelCenter, wheelRadius - 22.0f, Theme::WithAlpha(Color{ 255, 255, 255, 255 }, 0.10f));
        DrawCircleV(wheelCenter, wheelRadius - 70.0f, Theme::WithAlpha(Color{ 177, 156, 255, 255 }, 0.22f));
        for (int i = 4; i >= 1; --i) {
            DrawRing(wheelCenter, wheelRadius - 38.0f - i * 2.5f, wheelRadius - 34.0f, 0.0f, 360.0f, 100,
                     Theme::WithAlpha(Color{ 6, 182, 212, 255 }, 0.10f * (5 - i)));
        }
        DrawRing(wheelCenter, wheelRadius - 39.0f, wheelRadius - 34.0f, 0.0f, 360.0f, 100,
                 Theme::WithAlpha(Color{ 103, 232, 249, 255 }, 0.95f));
        DrawRing(wheelCenter, wheelRadius - 41.0f, wheelRadius - 39.0f, 0.0f, 360.0f, 100,
                 Theme::WithAlpha(WHITE, 0.65f));
        for (int i = 0; i < 12; ++i) {
            float a = (float)i * PI / 6.0f;
            DrawLineEx({ wheelCenter.x + std::cos(a) * 30.0f, wheelCenter.y + std::sin(a) * 30.0f },
                       { wheelCenter.x + std::cos(a) * (wheelRadius - 48.0f),
                         wheelCenter.y + std::sin(a) * (wheelRadius - 48.0f) },
                       1.2f, Theme::WithAlpha(Color{ 255, 255, 255, 255 }, 0.10f));
        }
        DrawCircleV(wheelCenter, 60.0f, Theme::WithAlpha(Color{ 255, 255, 255, 255 }, 0.10f));
        DrawCircleLines((int)wheelCenter.x, (int)wheelCenter.y, 60.0f, Theme::WithAlpha(WHITE, 0.20f));
        DrawCircleLines((int)wheelCenter.x, (int)wheelCenter.y, 48.0f, Theme::WithAlpha(WHITE, 0.12f));
        DrawEllipse((int)(wheelCenter.x - 45.0f), (int)(wheelCenter.y - 70.0f), 55.0f, 18.0f,
                    Theme::WithAlpha(WHITE, 0.12f));
    }

    void drawLaserLine() {
        if (selectedIndices.empty()) return;
        bool validPrefix = isPrefixOfUnsolved(currentWord());
        Color glow = validPrefix ? Color{ 6, 182, 212, 255 } : Color{ 244, 63, 94, 255 };
        Color mid = validPrefix ? Color{ 34, 211, 238, 255 } : Color{ 251, 113, 133, 255 };
        Color core = validPrefix ? Color{ 103, 232, 249, 255 } : Color{ 255, 205, 210, 255 };

        std::vector<Vector2> pts;
        for (int idx : selectedIndices) {
            if (idx >= 0 && idx < (int)letterPos.size()) pts.push_back(letterPos[idx]);
        }
        // Keep the thread connected to the cursor even between letters.
        if (dragging && !pts.empty()) pts.push_back(Input::Mouse());
        for (size_t i = 0; i + 1 < pts.size(); ++i) {
            DrawLineEx(pts[i], pts[i + 1], 22.0f, Theme::WithAlpha(glow, 0.10f));
            DrawLineEx(pts[i], pts[i + 1], 14.0f, Theme::WithAlpha(mid, 0.22f));
            DrawLineEx(pts[i], pts[i + 1], 8.0f, Theme::WithAlpha(core, 0.55f));
            DrawLineEx(pts[i], pts[i + 1], 4.0f, Theme::WithAlpha(WHITE, 1.0f));
        }
        for (size_t i = 0; i < selectedIndices.size(); ++i) {
            int idx = selectedIndices[i];
            if (idx < 0 || idx >= (int)letterPos.size()) continue;
            Vector2 p = letterPos[idx];
            DrawCircleV(p, 30.0f, Theme::WithAlpha(mid, 0.18f));
            DrawCircleLines((int)p.x, (int)p.y, 27.0f, Theme::WithAlpha(WHITE, 0.75f));
            if (i + 1 == selectedIndices.size()) {
                DrawCircleLines((int)p.x, (int)p.y, 33.0f, Theme::WithAlpha(core, 0.95f));
                DrawCircleV(p, 4.0f, WHITE);
            }
        }
    }

    void drawLetters() {
        float size = wheelLetters.size() > 14 ? 20.0f : (wheelLetters.size() > 11 ? 24.0f : (wheelLetters.size() > 8 ? 30.0f : 38.0f));
        Vector2 mouse = Input::Mouse();
        int hover = hitLetter(mouse);
        for (size_t i = 0; i < wheelLetters.size(); ++i) {
            Vector2 p = letterPos[i];
            bool selected = std::find(selectedIndices.begin(), selectedIndices.end(), (int)i) != selectedIndices.end();
            if ((int)i == hover && !selected) DrawCircleV(p, 22.0f, Theme::WithAlpha(WHITE, 0.16f));
            Color col = (i < wheelColors.size()) ? wheelColors[i] : Theme::gold;
            if (selected) {
                for (int g = 3; g >= 1; --g) {
                    DrawCircleV(p, 20.0f + g * 3.0f, Theme::WithAlpha(col, 0.10f * (4 - g)));
                }
            }
            std::string s(1, wheelLetters[i]);
            Theme::text3D(Theme::fontCartoon, s, { p.x, p.y + 1.0f }, size,
                          col, Color{ 35, 22, 10, 255 }, 2.2f,
                          Theme::WithAlpha(BLACK, 0.55f), true, true);
        }
    }

    void drawHeader() {
        UI::CoinsBadge({ 16.0f, 18.0f }, 340, 118.0f, 42.0f);
        Theme::text3D(Theme::fontCartoon, "LEVEL " + std::to_string(currentLevelIdx + 1) + ":",
                      { 270.0f, 30.0f }, 19.0f, Color{ 249, 168, 212, 255 }, WHITE, 1.2f,
                      Theme::WithAlpha(BLACK, 0.45f), true, true);
        Theme::text3D(Theme::fontCartoon, gGameData.unitShortTitle(gGameData.levels[currentLevelIdx].unit_index),
                      { 270.0f, 58.0f }, 34.0f, Color{ 253, 186, 58, 255 }, Color{ 120, 53, 15, 255 }, 2.6f,
                      Theme::WithAlpha(BLACK, 0.5f), true, true);
        Rectangle settings{ 462.0f, 16.0f, 52.0f, 52.0f };
        UI::IconButton3D(settings, Color{ 56, 189, 248, 255 }, Color{ 2, 132, 199, 255 }, Color{ 186, 230, 253, 255 });
        UI::GearIcon({ settings.x + 26.0f, settings.y + 26.0f }, 14.0f, WHITE);
    }

    void drawBottomButtons() {
        Vector2 sc{ 70.0f, 906.0f };
        DrawCircleV({ sc.x, sc.y + 4.0f }, 37.0f, Theme::WithAlpha(BLACK, 0.40f));
        DrawCircleGradient((int)sc.x, (int)sc.y, 36.0f, Color{ 56, 189, 248, 255 }, Color{ 3, 105, 161, 255 });
        DrawCircleLines((int)sc.x, (int)sc.y, 36.0f, Color{ 224, 242, 254, 255 });
        DrawCircleSector({ sc.x, sc.y - 3.0f }, 30.0f, 200.0f, 340.0f, 18, Theme::WithAlpha(WHITE, 0.18f));
        UI::ShuffleIcon(sc, 20.0f, WHITE);
        Vector2 hc{ 470.0f, 906.0f };
        DrawCircleV({ hc.x, hc.y + 4.0f }, 37.0f, Theme::WithAlpha(BLACK, 0.40f));
        DrawCircleGradient((int)hc.x, (int)hc.y, 36.0f, Color{ 254, 240, 138, 255 }, Color{ 245, 158, 11, 255 });
        DrawCircleLines((int)hc.x, (int)hc.y, 36.0f, Color{ 255, 251, 235, 255 });
        DrawCircleSector({ hc.x, hc.y - 3.0f }, 30.0f, 200.0f, 340.0f, 18, Theme::WithAlpha(WHITE, 0.25f));
        UI::BulbIcon(hc, 19.0f);
        DrawCircleV({ hc.x + 25.0f, hc.y - 24.0f }, 12.0f, Color{ 239, 68, 68, 255 });
        Theme::text3D(Theme::fontCartoon, "x" + std::to_string(hintsRemaining), { hc.x + 25.0f, hc.y - 23.0f },
                      11.0f, WHITE, Color{ 127, 29, 29, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.5f), true, true);
    }

    void drawHintToast() {
        if (hintToastTimer <= 0.0f) return;
        float a = std::min(1.0f, hintToastTimer / 0.35f);
        Rectangle r{ 150.0f, 108.0f, 240.0f, 44.0f };
        DrawRectangleRounded(r, 0.4f, 12, Theme::WithAlpha(Color{ 15, 23, 42, 242 }, a));
        DrawRectangleRoundedLines(r, 0.4f, 12, 1.4f, Theme::WithAlpha(Theme::gold, a * 0.8f));
        UI::TextArabic(hintToastText, { r.x + r.width * 0.5f, r.y + r.height * 0.5f }, 18.0f,
                       Theme::WithAlpha(Theme::goldLight, a), true, true);
    }

    void draw(int screenW, int screenH) {
        float sx = wrongFlash ? std::sin(time * 55.0f) * wrongShake : 0.0f;
        Art::SpaceBackdrop(screenW, screenH, time);
        drawWhiteGrid();
        drawPrompt();
        BeginScissorMode((int)sx, 0, screenW, screenH);
        drawWheelBackground();
        drawLaserLine();
        drawLetters();
        EndScissorMode();
        drawHeader();
        drawBottomButtons();
        drawHintToast();
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
            wrongShake *= 0.90f;
            if (wrongTimer <= 0.0f) clearSelection();
        }
        if (victoryModal.active) victoryModal.update(dt);
    }
};
