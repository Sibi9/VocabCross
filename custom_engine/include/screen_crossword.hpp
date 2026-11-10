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
#include <string>
#include <vector>

class ScreenCrossword {
public:
    struct TrayOrb {
        std::string letter;
        Vector2 pos;
        Color color = WHITE;
        Color shadow = BLACK;
        bool used = false;
        bool emptySocket = false;
    };

    struct Layout {
        float tile = 52.0f;
        float gap = 4.0f;
        float originX = 0.0f;
        float originY = 0.0f;
    };

    VictoryModal victoryModal;
    CrosswordPuzzle puzzle;
    std::vector<TrayOrb> orbs;
    Layout layout;

    int currentLevelIdx = 0;
    int activeWord = 0;
    int activeCell = -1;
    int hintsRemaining = 3;
    bool completed = false;
    bool wrongFlash = false;
    float wrongTimer = 0.0f;
    float time = 0.0f;
    float shake = 0.0f;

    void init() {
        victoryModal.init();
        loadLevel(gGameData.current_level);
    }

    void unload() {}

    void loadLevel(int levelIdx) {
        currentLevelIdx = std::clamp(levelIdx, 0, std::max(0, gGameData.levelCount() - 1));
        if (gGameData.levelCount() == 0) return;
        auto& level = gGameData.levels[currentLevelIdx];
        puzzle.build(level.data, gGameData.units[level.unit_index]);
        completed = false;
        wrongFlash = false;
        wrongTimer = 0.0f;
        hintsRemaining = 3;
        victoryModal.hide();
        activeWord = 0;
        activeCell = puzzle.firstEmptyCell(activeWord);
        computeLayout();
        buildTray();
    }

    void resetStage() { loadLevel(gGameData.current_level); }

    void computeLayout() {
        int cols = std::max(1, puzzle.cols());
        int rows = std::max(1, puzzle.rows());
        float gap = 4.0f;
        // Keep the left 150px free for the fox + hint, so no cell is hidden.
        float availW = 376.0f;
        float availH = 520.0f;
        float tile = std::min(52.0f, (availW - gap * (cols - 1)) / cols);
        tile = std::min(tile, (availH - gap * (rows - 1)) / rows);
        tile = std::max(26.0f, tile);
        float totalW = cols * tile + (cols - 1) * gap;
        float totalH = rows * tile + (rows - 1) * gap;
        layout.tile = tile;
        layout.gap = gap;
        layout.originX = 150.0f + (availW - totalW) * 0.5f;
        layout.originY = 385.0f - totalH * 0.5f;
        if (layout.originY < 120.0f) layout.originY = 120.0f;
    }

    Rectangle cellRect(const PuzzleCell& c) const {
        float step = layout.tile + layout.gap;
        return { layout.originX + c.col * step, layout.originY + c.row * step, layout.tile, layout.tile };
    }

    void buildTray() {
        orbs.clear();
        std::vector<char> letters = puzzle.supplyLetters();
        const std::string decoys = "AEIOUNRSTL";
        int di = 0;
        while ((int)letters.size() < 18 && di < (int)decoys.size()) {
            letters.push_back(decoys[di++]);
        }
        unsigned seed = (unsigned)currentLevelIdx * 2654435761u + 73u;
        for (size_t i = letters.size(); i > 1; --i) {
            seed = seed * 1664525u + 1013904223u;
            size_t j = seed % i;
            std::swap(letters[i - 1], letters[j]);
        }
        static const Color palette[] = {
            { 253, 224, 71, 255 }, { 34, 197, 94, 255 }, { 245, 158, 11, 255 },
            { 249, 115, 22, 255 }, { 239, 68, 68, 255 }, { 168, 85, 247, 255 },
            { 244, 63, 94, 255 }, { 56, 189, 248, 255 }, { 59, 130, 246, 255 },
            { 168, 85, 247, 255 }, { 236, 72, 153, 255 }, { 244, 114, 182, 255 },
            { 203, 213, 225, 255 }, { 251, 146, 60, 255 }, { 45, 212, 191, 255 },
            { 129, 140, 248, 255 }, { 250, 204, 21, 255 }, { 74, 222, 128, 255 }
        };
        static const Color shadows[] = {
            { 202, 138, 4, 255 }, { 21, 128, 61, 255 }, { 180, 83, 9, 255 },
            { 194, 65, 12, 255 }, { 185, 28, 28, 255 }, { 126, 34, 206, 255 },
            { 190, 24, 93, 255 }, { 2, 132, 199, 255 }, { 29, 78, 216, 255 },
            { 126, 34, 206, 255 }, { 190, 24, 93, 255 }, { 219, 39, 119, 255 },
            { 148, 163, 184, 255 }, { 194, 65, 12, 255 }, { 17, 94, 89, 255 },
            { 49, 46, 129, 255 }, { 202, 138, 4, 255 }, { 22, 163, 74, 255 }
        };
        for (int i = 0; i < 18; ++i) {
            TrayOrb o;
            int row = i / 9;
            int col = i % 9;
            o.pos = { 64.0f + col * 51.0f, row == 0 ? 872.0f : 924.0f };
            o.color = palette[i % 18];
            o.shadow = shadows[i % 18];
            if (i < (int)letters.size()) o.letter = std::string(1, letters[i]);
            else o.emptySocket = true;
            orbs.push_back(o);
        }
    }

    void useOrbFor(char c) {
        for (auto& o : orbs) {
            if (!o.used && !o.emptySocket && !o.letter.empty() && o.letter[0] == c) {
                o.used = true;
                return;
            }
            if (!o.emptySocket && o.letter.empty() && !o.used) {
                o.used = true;
                return;
            }
        }
    }

    void restoreOrbFor(char c) {
        for (auto& o : orbs) {
            if (o.used && !o.emptySocket && !o.letter.empty() && o.letter[0] == c) {
                o.used = false;
                return;
            }
        }
    }

    int activeCellOrNext() {
        if (activeWord < 0 || activeWord >= (int)puzzle.words.size()) activeWord = 0;
        if (activeCell >= 0 && activeCell < (int)puzzle.cells.size() &&
            puzzle.cells[activeCell].entered == '_' && !puzzle.cells[activeCell].locked) {
            return activeCell;
        }
        int ci = puzzle.firstEmptyCell(activeWord);
        if (ci >= 0) return ci;
        int next = puzzle.nextUnsolvedWord(activeWord);
        if (next >= 0) {
            activeWord = next;
            return puzzle.firstEmptyCell(activeWord);
        }
        return -1;
    }

    void afterEntry() {
        int beforeSolved = 0;
        for (const auto& w : puzzle.words) beforeSolved += w.solved ? 1 : 0;
        puzzle.markSolvedWords();
        int afterSolved = 0;
        for (const auto& w : puzzle.words) afterSolved += w.solved ? 1 : 0;
        if (afterSolved > beforeSolved) gAudio.playChime();
        if (puzzle.allSolved()) {
            complete();
            return;
        }
        if (activeWord >= 0 && activeWord < (int)puzzle.words.size() && puzzle.words[activeWord].solved) {
            int next = puzzle.nextUnsolvedWord(activeWord);
            if (next >= 0) {
                activeWord = next;
                activeCell = puzzle.firstEmptyCell(activeWord);
            }
        } else {
            activeCell = activeCellOrNext();
        }
    }

    void fillFromOrb(int orbIndex) {
        if (orbIndex < 0 || orbIndex >= (int)orbs.size()) return;
        auto& o = orbs[orbIndex];
        if (o.used || o.emptySocket || o.letter.empty()) return;
        int ci = activeCellOrNext();
        if (ci < 0) return;
        puzzle.cells[ci].entered = o.letter[0];
        o.used = true;
        activeWord = puzzle.firstWordAtCell(ci);
        activeCell = puzzle.firstEmptyCell(activeWord);
        gAudio.playPop();
        afterEntry();
    }

    void removeCellLetter(int ci) {
        if (ci < 0 || ci >= (int)puzzle.cells.size()) return;
        if (puzzle.cells[ci].locked || puzzle.cells[ci].entered == '_') return;
        restoreOrbFor(puzzle.cells[ci].entered);
        puzzle.cells[ci].entered = '_';
        activeWord = puzzle.firstWordAtCell(ci);
        activeCell = ci;
        for (auto& w : puzzle.words) w.wrong = false;
        puzzle.markSolvedWords();
        gAudio.playPop();
    }

    void hint() {
        if (completed || hintsRemaining <= 0) return;
        int wi = activeWord;
        if (wi < 0 || wi >= (int)puzzle.words.size() || puzzle.words[wi].solved) {
            int next = puzzle.nextUnsolvedWord(activeWord);
            if (next >= 0) wi = next;
        }
        int ci = puzzle.firstEmptyCell(wi);
        if (ci < 0) return;
        activeWord = wi;
        puzzle.cells[ci].entered = puzzle.cells[ci].answer;
        useOrbFor(puzzle.cells[ci].answer);
        hintsRemaining--;
        gAudio.playChime();
        activeCell = puzzle.firstEmptyCell(wi);
        afterEntry();
    }

    void revealAll() {
        if (completed) return;
        puzzle.solveAll();
        gAudio.playFanfare();
        complete();
    }

    void complete() {
        if (completed) return;
        completed = true;
        gGameData.completeLevel(currentLevelIdx, 3);
        victoryModal.show(gGameData.levels[currentLevelIdx].data);
    }

    std::string patternForWord(int wi) const {
        if (wi < 0 || wi >= (int)puzzle.words.size()) return "";
        const std::string& w = puzzle.words[wi].answer;
        if (w.size() <= 2) return "(" + w + ")";
        std::string s = "(" + std::string(1, w.front()) + " ";
        for (size_t i = 1; i + 1 < w.size(); ++i) s += "_ ";
        s += std::string(1, w.back()) + ")";
        return s;
    }

    Rectangle wordChipRect(int wi) const {
        int n = (int)puzzle.words.size();
        float w = 112.0f, h = 28.0f, gap = 8.0f;
        float total = n * w + (n - 1) * gap;
        float x = (540.0f - total) * 0.5f + wi * (w + gap);
        return { x, 658.0f, w, h };
    }

    void selectWord(int wi) {
        if (wi < 0 || wi >= (int)puzzle.words.size()) return;
        activeWord = wi;
        activeCell = puzzle.firstEmptyCell(wi);
        gAudio.playClick();
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

        Rectangle gear{ 340.0f, 12.0f, 52.0f, 52.0f };
        if (released && CheckCollisionPointRec(mouse, gear)) {
            gAudio.playClick();
            nextScreen = Nav::SCREEN_MAP;
            return;
        }
        if (released && CheckCollisionPointCircle(mouse, { 65.0f, 632.0f }, 32.0f)) {
            hint();
            return;
        }
        if (released && CheckCollisionPointCircle(mouse, { 456.0f, 796.0f }, 30.0f)) {
            revealAll();
            return;
        }
        if (released) {
            for (int wi = 0; wi < (int)puzzle.words.size(); ++wi) {
                if (CheckCollisionPointRec(mouse, wordChipRect(wi))) {
                    selectWord(wi);
                    return;
                }
            }
            for (const auto& c : puzzle.cells) {
                if (!CheckCollisionPointRec(mouse, cellRect(c))) continue;
                int ci = puzzle.cellIndex(c.row, c.col);
                if (ci < 0) return;
                if (puzzle.cells[ci].entered != '_') removeCellLetter(ci);
                else {
                    activeWord = puzzle.firstWordAtCell(ci);
                    activeCell = ci;
                    gAudio.playClick();
                }
                return;
            }
            for (int i = 0; i < (int)orbs.size(); ++i) {
                if (CheckCollisionPointCircle(mouse, orbs[i].pos, 22.0f)) {
                    fillFromOrb(i);
                    return;
                }
            }
        }

        int key = GetKeyPressed();
        while (key > 0) {
            if (key >= 'A' && key <= 'Z') {
                for (int i = 0; i < (int)orbs.size(); ++i) {
                    if (!orbs[i].used && !orbs[i].emptySocket && !orbs[i].letter.empty() &&
                        orbs[i].letter[0] == (char)key) {
                        fillFromOrb(i);
                        break;
                    }
                }
            }
            key = GetKeyPressed();
        }
    }

    void drawBackground() {
        DrawRectangleGradientV(0, 0, 540, 960, Color{ 174, 218, 245, 255 }, Color{ 198, 236, 250, 255 });
        DrawRectangleGradientV(0, 470, 540, 490, Color{ 198, 236, 250, 255 }, Color{ 153, 213, 201, 255 });
        Art::Cloud({ 90, 150 }, 150, Color{ 255, 255, 255, 255 }, 0.85f);
        Art::Cloud({ 430, 96 }, 120, Color{ 255, 255, 255, 255 }, 0.75f);
        Art::Cloud({ 510, 330 }, 110, Color{ 255, 255, 255, 255 }, 0.6f);
        Art::Cloud({ 40, 700 }, 150, Color{ 255, 255, 255, 255 }, 0.55f);
        Art::GrassPatch({ -40, 130, 210, 130 }, Color{ 110, 231, 183, 255 }, Color{ 16, 185, 129, 255 });
        Art::Building({ 66, 222 }, 0.74f, Color{ 241, 245, 249, 255 }, Color{ 203, 213, 225, 255 }, Color{ 56, 189, 248, 255 });
        Art::Building({ 128, 206 }, 0.52f, Color{ 226, 232, 240, 255 }, Color{ 148, 163, 184, 255 }, Color{ 125, 211, 252, 255 });
        Art::ScalesOfJustice({ 360, 170 }, 0.80f);
        Art::Handshake({ 498, 192 }, 0.84f);
        Art::GrassPatch({ -55, 390, 180, 120 }, Color{ 110, 231, 183, 255 }, Color{ 16, 185, 129, 255 });
        Art::LawBookProps({ 44, 438 }, 0.90f);
        Art::PapersProp({ 508, 410 }, 0.90f);
        Art::BriefcaseProp({ 508, 690 }, 0.86f);
        Art::GrassPatch({ 460, 600, 120, 120 }, Color{ 110, 231, 183, 255 }, Color{ 16, 185, 129, 255 });
        Art::Building({ 510, 650 }, 0.46f, Color{ 241, 245, 249, 255 }, Color{ 203, 213, 225, 255 }, Color{ 96, 165, 250, 255 });
        for (int i = 0; i < 12; ++i) {
            UI::Sparkle({ 25.0f + (i * 83) % 500, 300.0f + (i * 57) % 380 }, 2.5f,
                        Theme::WithAlpha(WHITE, 0.35f + 0.2f * std::sin(time * 2.0f + i)));
        }
    }

    void drawHeader() {
        Rectangle header{ 15.0f, 12.0f, 306.0f, 56.0f };
        UI::Capsule(header, Color{ 220, 241, 252, 245 }, Color{ 174, 218, 245, 245 }, WHITE);
        Theme::text3D(Theme::fontCartoon, "LEVEL " + std::to_string(currentLevelIdx + 1) + " - CROSSWORD",
                      { header.x + 20.0f, header.y + 19.0f }, 15.0f, Theme::goldLight,
                      Color{ 120, 60, 25, 255 }, 1.6f, Theme::WithAlpha(Color{ 60, 30, 10, 255 }, 0.8f));
        Theme::text3D(Theme::fontCartoon, gGameData.unitQuestTitle(gGameData.levels[currentLevelIdx].unit_index),
                      { header.x + 20.0f, header.y + 34.0f }, 29.0f,
                      Color{ 251, 146, 60, 255 }, Color{ 109, 59, 26, 255 }, 2.6f,
                      Theme::WithAlpha(Color{ 67, 20, 7, 255 }, 0.9f));

        Rectangle gear{ 340.0f, 12.0f, 52.0f, 52.0f };
        UI::IconButton3D(gear, Color{ 205, 128, 72, 255 }, Color{ 140, 82, 43, 255 }, Color{ 245, 158, 11, 255 });
        UI::GearIcon({ gear.x + 26.0f, gear.y + 26.0f }, 14.0f, Color{ 255, 253, 245, 255 });
        UI::CoinsBadge({ 408.0f, 12.0f }, 450, 118.0f, 38.0f);
        UI::GemsBadge({ 408.0f, 56.0f }, 12, 118.0f, 38.0f);
    }

    void drawGrid() {
        // Gold glow around every unsolved word's cells, green for solved.
        for (const auto& c : puzzle.cells) {
            Rectangle r = cellRect(c);
            if (c.locked) {
                continue;
            }
            bool inWord = false;
            for (int wi : c.words) if (!puzzle.words[wi].solved) { inWord = true; break; }
            if (inWord) {
                DrawRectangleRounded({ r.x - 3.0f, r.y - 3.0f, r.width + 6.0f, r.height + 6.0f },
                                     UI::Roundness(r, 12.0f), 12, Theme::WithAlpha(Theme::gold, 0.19f));
            }
        }
        std::vector<int> startNumbers(puzzle.cells.size(), 0);
        for (int wi = 0; wi < (int)puzzle.words.size(); ++wi) {
            auto indices = puzzle.wordCellIndices(wi);
            if (!indices.empty()) startNumbers[indices.front()] = wi + 1;
        }
        for (const auto& c : puzzle.cells) {
            Rectangle r = cellRect(c);
            int ci = puzzle.cellIndex(c.row, c.col);
            bool locked = c.locked;
            bool active = (ci == activeCell);
            std::string letter = (c.entered == '_') ? "" : std::string(1, c.entered);
            UI::drawWoodenTile3D(r, active, letter.empty(), letter, false);
            if (locked) {
                DrawRectangleRoundedLines({ r.x - 1.0f, r.y - 1.0f, r.width + 2.0f, r.height + 2.0f },
                                          UI::Roundness(r, 12.0f), 12, 2.2f, Color{ 74, 222, 128, 255 });
            }
            if (ci >= 0 && ci < (int)startNumbers.size() && startNumbers[ci] > 0) {
                Theme::text3D(Theme::fontCartoon, std::to_string(startNumbers[ci]),
                              { r.x + 5.0f, r.y + 3.0f }, 9.0f, Theme::goldLight,
                              Color{ 76, 40, 20, 255 }, 0.8f, Theme::WithAlpha(BLACK, 0.5f));
            }
        }
        // active word ribbon
        if (activeWord >= 0 && activeWord < (int)puzzle.words.size()) {
            const auto& w = puzzle.words[activeWord];
            float step = layout.tile + layout.gap;
            Rectangle ar{ layout.originX + w.col * step - 7.0f,
                          layout.originY + w.row * step - 7.0f,
                          (w.horizontal ? (float)w.answer.size() : 1.0f) * step - layout.gap + 14.0f,
                          (w.horizontal ? 1.0f : (float)w.answer.size()) * step - layout.gap + 14.0f };
            DrawRectangleRoundedLines(ar, 0.3f, 14, 2.6f, Theme::WithAlpha(Theme::goldLight, 0.85f));
        }
    }

    void drawWordChips() {
        for (int wi = 0; wi < (int)puzzle.words.size(); ++wi) {
            Rectangle r = wordChipRect(wi);
            bool active = (wi == activeWord);
            bool solved = puzzle.words[wi].solved;
            bool wrong = puzzle.words[wi].wrong;
            Color top = solved ? Color{ 74, 222, 128, 255 } : (wrong ? Color{ 248, 113, 113, 255 } : (active ? Color{ 251, 191, 36, 255 } : Color{ 30, 43, 68, 240 }));
            Color bottom = solved ? Color{ 22, 163, 74, 255 } : (wrong ? Color{ 185, 28, 28, 255 } : (active ? Color{ 217, 119, 6, 255 } : Color{ 13, 22, 40, 245 }));
            UI::Capsule(r, top, bottom, active ? Theme::goldLight : Theme::WithAlpha(WHITE, 0.35f));
            std::string label = std::to_string(wi + 1) + " · " + std::to_string(puzzle.words[wi].answer.size());
            Theme::text3D(Theme::fontCartoon, label, { r.x + r.width * 0.5f, r.y + r.height * 0.5f },
                          12.0f, WHITE, Color{ 22, 30, 50, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.45f), true, true);
            if (solved) UI::CheckBadge({ r.x + r.width - 6.0f, r.y + 6.0f }, 7.0f);
        }
    }

    void drawFoxAndHint() {
        Art::FoxMascot({ 68.0f, 602.0f }, 1.12f);
        Vector2 hc{ 65.0f, 632.0f };
        DrawCircleV({ hc.x, hc.y + 4.0f }, 32.0f, Theme::WithAlpha(BLACK, 0.35f));
        UI::GradientRound({ hc.x - 31.0f, hc.y - 31.0f, 62.0f, 62.0f }, 31.0f,
                          Color{ 254, 240, 138, 255 }, Color{ 245, 158, 11, 255 });
        DrawCircleLines((int)hc.x, (int)hc.y, 31.0f, Theme::goldLight);
        UI::MagnifierIcon({ hc.x, hc.y }, 15.0f, Color{ 120, 60, 25, 255 }, Color{ 125, 211, 252, 255 });
        DrawCircleV({ hc.x + 24.0f, hc.y - 25.0f }, 12.0f, Color{ 239, 68, 68, 255 });
        Theme::text3D(Theme::fontCartoon, "x" + std::to_string(hintsRemaining), { hc.x + 24.0f, hc.y - 24.0f },
                      12.0f, WHITE, Color{ 127, 29, 29, 255 }, 1.0f, Theme::WithAlpha(BLACK, 0.5f), true, true);
        Theme::text3D(Theme::fontCartoon, "HINT", { hc.x, hc.y + 44.0f }, 15.0f,
                      Theme::goldLight, Color{ 109, 59, 26, 255 }, 1.6f, Theme::WithAlpha(BLACK, 0.6f), true, true);
    }

    void drawDefinition() {
        Rectangle def{ 24.0f, 694.0f, 492.0f, 124.0f };
        UI::DropShadow(def, 20.0f, 5.0f, 0.26f, 1.0f);
        UI::GradientRound(def, 20.0f, Color{ 176, 203, 218, 238 }, Color{ 139, 170, 190, 242 });
        DrawRectangleRoundedLines(def, UI::Roundness(def, 20.0f), 16, 2.4f, WHITE);
        DrawRectangleRoundedLines({ def.x + 3.0f, def.y + 3.0f, def.width - 6.0f, def.height - 6.0f },
                                  UI::Roundness(def, 17.0f), 16, 1.0f, Theme::WithAlpha(WHITE, 0.55f));
        Rectangle gloss{ def.x + 7.0f, def.y + 5.0f, def.width - 14.0f, def.height * 0.30f };
        DrawRectangleRounded(gloss, UI::Roundness(gloss, 14.0f), 10, Theme::WithAlpha(WHITE, 0.15f));

        int wi = std::clamp(activeWord, 0, std::max(0, (int)puzzle.words.size() - 1));
        const WordData& data = puzzle.words.empty() ? gGameData.levels[currentLevelIdx].data : puzzle.words[wi].data;
        std::string header = "DEFINITION:  WORD " + std::to_string(wi + 1) + " / " + std::to_string(puzzle.words.size());
        Theme::text3D(Theme::fontCartoon, header, { def.x + def.width * 0.5f, def.y + 19.0f }, 15.0f,
                      Color{ 250, 204, 21, 255 }, Color{ 109, 59, 26, 255 }, 1.6f,
                      Theme::WithAlpha(Color{ 60, 30, 10, 255 }, 0.8f), true, true);
        auto lines = UI::WrapText(data.definition_en, 19.0f, 350.0f);
        if (lines.size() > 2) lines = UI::WrapText(data.definition_en, 17.0f, 350.0f);
        float size = (lines.size() > 2) ? 17.0f : 19.0f;
        float y = def.y + 41.0f;
        for (size_t i = 0; i < lines.size() && i < 2; ++i) {
            Theme::text3D(Theme::fontCartoon, lines[i], { def.x + def.width * 0.5f, y }, size,
                          WHITE, Color{ 30, 41, 59, 255 }, 1.5f, Theme::WithAlpha(BLACK, 0.5f), true, false);
            y += size + 4.0f;
        }
        Theme::text3D(Theme::fontCartoon, patternForWord(wi), { def.x + def.width * 0.5f, def.y + 104.0f }, 17.0f,
                      Theme::goldLight, Color{ 109, 59, 26, 255 }, 1.5f, Theme::WithAlpha(BLACK, 0.6f), true, true);
    }

    void drawReveal() {
        Vector2 c{ 456.0f, 796.0f };
        DrawCircleV({ c.x, c.y + 4.0f }, 30.0f, Theme::WithAlpha(BLACK, 0.35f));
        UI::GradientRound({ c.x - 29.0f, c.y - 29.0f, 58.0f, 58.0f }, 29.0f,
                          Color{ 254, 240, 138, 255 }, Color{ 245, 158, 11, 255 });
        DrawCircleLines((int)c.x, (int)c.y, 29.0f, Theme::goldLight);
        UI::BulbIcon(c, 16.0f);
        Theme::text3D(Theme::fontCartoon, "REVEAL", { c.x, c.y + 42.0f }, 13.0f,
                      Theme::goldLight, Color{ 109, 59, 26, 255 }, 1.4f, Theme::WithAlpha(BLACK, 0.6f), true, true);
    }

    void drawTray() {
        Rectangle tray{ 14.0f, 842.0f, 512.0f, 108.0f };
        UI::drawWoodenTray(tray);
        for (const auto& o : orbs) UI::TrayHole(o.pos, 21.0f);
        for (const auto& o : orbs) {
            if (o.emptySocket) {
                DrawCircleV(o.pos, 20.0f, Color{ 58, 28, 11, 255 });
                DrawCircleV(o.pos, 17.0f, Color{ 76, 38, 17, 255 });
                DrawCircleLines((int)o.pos.x, (int)o.pos.y, 19.0f, Theme::WithAlpha(Theme::gold, 0.25f));
            } else if (!o.used) {
                UI::drawCandyOrb(o.pos, 21.0f, o.color, o.shadow, o.letter);
            }
        }
    }

    void draw(int screenW, int screenH) {
        float shakeX = wrongFlash ? std::sin(time * 55.0f) * shake : 0.0f;
        DrawRectangle(0, 0, screenW, screenH, Color{ 188, 227, 250, 255 });
        BeginScissorMode((int)shakeX, 0, screenW, screenH);
        drawBackground();
        drawHeader();
        drawGrid();
        drawWordChips();
        drawFoxAndHint();
        drawDefinition();
        drawTray();
        drawReveal();
        Theme::text3D(Theme::fontCartoon, "TAP A LETTER  •  TAP A CELL TO REMOVE",
                      { 270.0f, 826.0f }, 11.0f, Theme::goldLight,
                      Color{ 76, 40, 20, 255 }, 1.2f, Theme::WithAlpha(BLACK, 0.55f), true, true);
        EndScissorMode();

        Theme::drawParticles();
        if (victoryModal.active) {
            bool n = false, m = false;
            victoryModal.draw(gGameData.levels[currentLevelIdx], screenW, screenH, n, m);
        }
    }

    void update(float dt) {
        time += dt;
        if (wrongFlash) {
            wrongTimer -= dt;
            shake *= 0.90f;
            if (wrongTimer <= 0.0f) {
                wrongFlash = false;
                shake = 0.0f;
            }
        }
        if (victoryModal.active) victoryModal.update(dt);
    }
};
