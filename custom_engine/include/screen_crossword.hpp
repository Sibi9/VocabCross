#pragma once

#include "raylib.h"
#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "decor_art.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"
#include "ui_widgets.hpp"
#include "victory_modal.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

class ScreenCrossword {
public:
    struct TrayOrb {
        std::string letter;
        Vector2 pos{};
        bool used = false;
    };

    struct CrossWordItem {
        WordData data;
        std::string answer;
        int row = 0;
        int col = 0;
        bool horizontal = true;
        bool solved = false;
    };

    struct GridCell {
        int row = 0;
        int col = 0;
        char answer = 'A';
        char entered = '_';
        bool locked = false;
        int trayOrbIdx = -1;
        std::vector<int> wordIndices;
    };

    VictoryModal victoryModal;
    std::vector<CrossWordItem> words;
    std::vector<GridCell> cells;
    int minRow = 0, maxRow = 0, minCol = 0, maxCol = 0;
    int activeWord = 0;

    std::vector<TrayOrb> trayOrbs;
    std::vector<int> typedCellHistory;
    int levelIndex = 0;
    bool levelCompleted = false;
    float wrongFlashTimer = 0.0f;

    void init() {
        victoryModal.init();
        setLevel(0);
    }

    GridCell* getCell(int r, int c) {
        for (auto& cell : cells) {
            if (cell.row == r && cell.col == c) return &cell;
        }
        return nullptr;
    }

    const GridCell* getCell(int r, int c) const {
        for (const auto& cell : cells) {
            if (cell.row == r && cell.col == c) return &cell;
        }
        return nullptr;
    }

    bool canPlaceWord(const std::string& ans, int r, int c, bool horiz) const {
        for (size_t i = 0; i < ans.size(); ++i) {
            int cr = horiz ? r : r + (int)i;
            int cc = horiz ? c + (int)i : c;
            const GridCell* existing = getCell(cr, cc);
            if (existing && existing->answer != ans[i]) return false;
        }
        return true;
    }

    void addWordToCells(int wIdx) {
        const auto& w = words[wIdx];
        for (size_t i = 0; i < w.answer.size(); ++i) {
            int r = w.horizontal ? w.row : w.row + (int)i;
            int c = w.horizontal ? w.col + (int)i : w.col;
            GridCell* cell = getCell(r, c);
            if (!cell) {
                GridCell nc;
                nc.row = r;
                nc.col = c;
                nc.answer = (char)std::toupper((unsigned char)w.answer[i]);
                nc.entered = '_';
                nc.locked = false;
                nc.trayOrbIdx = -1;
                nc.wordIndices.push_back(wIdx);
                cells.push_back(nc);
            } else {
                cell->wordIndices.push_back(wIdx);
            }
        }
    }

    void buildCrosswordGrid(const LevelInfo& lvl) {
        words.clear();
        cells.clear();
        activeWord = 0;

        // 1. Primary target word placed horizontally at (0, 0)
        CrossWordItem w0;
        w0.data = lvl.word;
        w0.answer.clear();
        for (char c : lvl.word.word) {
            if (std::isalpha((unsigned char)c)) w0.answer.push_back((char)std::toupper((unsigned char)c));
        }
        if (w0.answer.empty()) w0.answer = "FAKE";
        w0.row = 0;
        w0.col = 0;
        w0.horizontal = true;
        w0.solved = false;
        words.push_back(w0);
        addWordToCells(0);

        // 2. Find 1 or 2 real intersecting companion words from the same BAC unit
        std::vector<WordData> candidates;
        for (const auto& other : gGameData.levels) {
            if (other.unit == lvl.unit && other.word.word != lvl.word.word && !other.word.word.empty()) {
                candidates.push_back(other.word);
            }
        }
        for (const auto& other : gGameData.levels) {
            if (other.word.word != lvl.word.word && !other.word.word.empty()) {
                candidates.push_back(other.word);
            }
        }

        for (const auto& cand : candidates) {
            if (words.size() >= 3) break;
            std::string ans;
            for (char c : cand.word) {
                if (std::isalpha((unsigned char)c)) ans.push_back((char)std::toupper((unsigned char)c));
            }
            if (ans.size() < 3 || ans.size() > 7) continue;
            bool dup = false;
            for (const auto& ew : words) {
                if (ew.answer == ans) { dup = true; break; }
            }
            if (dup) continue;

            // Try intersecting with w0 (vertically) or w1 (horizontally)
            bool placed = false;
            for (size_t pwIdx = 0; pwIdx < words.size() && !placed; ++pwIdx) {
                const auto& pw = words[pwIdx];
                bool horiz = !pw.horizontal;
                for (size_t i = 0; i < pw.answer.size() && !placed; ++i) {
                    for (size_t j = 0; j < ans.size() && !placed; ++j) {
                        if (pw.answer[i] == ans[j]) {
                            int r = pw.horizontal ? pw.row - (int)j : pw.row + (int)i;
                            int c = pw.horizontal ? pw.col + (int)i : pw.col - (int)j;
                            if (canPlaceWord(ans, r, c, horiz)) {
                                CrossWordItem nw;
                                nw.data = cand;
                                nw.answer = ans;
                                nw.row = r;
                                nw.col = c;
                                nw.horizontal = horiz;
                                nw.solved = false;
                                int newIdx = (int)words.size();
                                words.push_back(nw);
                                addWordToCells(newIdx);
                                placed = true;
                            }
                        }
                    }
                }
            }
        }

        // Compute bounds
        minRow = maxRow = 0;
        minCol = maxCol = 0;
        for (const auto& c : cells) {
            minRow = std::min(minRow, c.row);
            maxRow = std::max(maxRow, c.row);
            minCol = std::min(minCol, c.col);
            maxCol = std::max(maxCol, c.col);
        }

        // Pre-fill companion words (words[1..]) EXCEPT cells that belong to the primary target word (words[0]),
        // AND pre-lock the first and last letters of words[0] if length >= 4 so the player has real locked hints!
        for (size_t wi = 1; wi < words.size(); ++wi) {
            const auto& w = words[wi];
            for (size_t i = 0; i < w.answer.size(); ++i) {
                int r = w.horizontal ? w.row : w.row + (int)i;
                int c = w.horizontal ? w.col + (int)i : w.col;
                GridCell* cell = getCell(r, c);
                if (cell) {
                    bool inPrimary = std::find(cell->wordIndices.begin(), cell->wordIndices.end(), 0) != cell->wordIndices.end();
                    if (!inPrimary) {
                        cell->entered = cell->answer;
                        cell->locked = true;
                    }
                }
            }
            words[wi].solved = true;
        }

        // For the primary target word (words[0]), lock first & last letters as true hints (so clicking A & K fills the middle slots!)
        const auto& mainW = words[0];
        if (mainW.answer.size() >= 4) {
            GridCell* firstCell = getCell(mainW.row, mainW.col);
            int lastR = mainW.horizontal ? mainW.row : mainW.row + (int)mainW.answer.size() - 1;
            int lastC = mainW.horizontal ? mainW.col + (int)mainW.answer.size() - 1 : mainW.col;
            GridCell* lastCell = getCell(lastR, lastC);
            if (firstCell) {
                firstCell->entered = firstCell->answer;
                firstCell->locked = true;
            }
            if (lastCell) {
                lastCell->entered = lastCell->answer;
                lastCell->locked = true;
            }
        }
    }

    void setLevel(int idx) {
        if (gGameData.levels.empty()) return;
        levelIndex = std::clamp(idx, 0, (int)gGameData.levels.size() - 1);
        const auto& lvl = gGameData.levels[levelIndex];

        buildCrosswordGrid(lvl);
        levelCompleted = false;
        wrongFlashTimer = 0.0f;
        victoryModal.hide();
        buildTrayOrbs();
    }

    void buildTrayOrbs() {
        trayOrbs.clear();
        typedCellHistory.clear();
        if (words.empty()) return;

        const auto& w = words[activeWord];
        std::vector<char> pool;
        // Include all unlocked letters needed for the active word
        for (size_t i = 0; i < w.answer.size(); ++i) {
            int r = w.horizontal ? w.row : w.row + (int)i;
            int c = w.horizontal ? w.col + (int)i : w.col;
            const GridCell* cell = getCell(r, c);
            if (cell && !cell->locked) {
                pool.push_back(cell->answer);
            }
        }

        const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        while (pool.size() < 14) {
            char extra = alphabet[(pool.size() * 7 + levelIndex * 3 + 5) % 26];
            pool.push_back(extra);
        }

        // Deterministic shuffle
        for (size_t i = 0; i < pool.size(); ++i) {
            size_t j = (i * 5 + levelIndex * 3 + 3) % pool.size();
            std::swap(pool[i], pool[j]);
        }

        for (int i = 0; i < 14; ++i) {
            int row = i / 7;
            int col = i % 7;
            TrayOrb orb;
            orb.letter = std::string(1, pool[i]);
            orb.pos = { 63.0f + col * 69.0f, 836.0f + row * 56.0f };
            orb.used = false;
            trayOrbs.push_back(orb);
        }
    }

    void checkWordCompletion() {
        if (words.empty()) return;
        auto& w = words[activeWord];

        bool allFilled = true;
        bool allCorrect = true;
        for (size_t i = 0; i < w.answer.size(); ++i) {
            int r = w.horizontal ? w.row : w.row + (int)i;
            int c = w.horizontal ? w.col + (int)i : w.col;
            const GridCell* cell = getCell(r, c);
            if (!cell || cell->entered == '_') {
                allFilled = false;
                break;
            }
            if (cell->entered != cell->answer) {
                allCorrect = false;
            }
        }

        if (!allFilled) return;

        if (allCorrect) {
            for (size_t i = 0; i < w.answer.size(); ++i) {
                int r = w.horizontal ? w.row : w.row + (int)i;
                int c = w.horizontal ? w.col + (int)i : w.col;
                GridCell* cell = getCell(r, c);
                if (cell) {
                    cell->entered = cell->answer;
                    cell->locked = true;
                }
            }
            w.solved = true;
            levelCompleted = true;
            gGameData.completeLevel(levelIndex, 3);
            victoryModal.show(gGameData.levels[levelIndex].word);
        } else {
            // Wrong guess: flash red briefly and return non-locked letters to the tray!
            wrongFlashTimer = 0.45f;
            gAudio.playPop();
            for (size_t i = 0; i < w.answer.size(); ++i) {
                int r = w.horizontal ? w.row : w.row + (int)i;
                int c = w.horizontal ? w.col + (int)i : w.col;
                GridCell* cell = getCell(r, c);
                if (cell && !cell->locked) {
                    if (cell->trayOrbIdx >= 0 && cell->trayOrbIdx < (int)trayOrbs.size()) {
                        trayOrbs[cell->trayOrbIdx].used = false;
                    }
                    cell->entered = '_';
                    cell->trayOrbIdx = -1;
                }
            }
            typedCellHistory.clear();
        }
    }

    void typeLetter(char ch, int orbIdx = -1) {
        if (words.empty()) return;
        const auto& w = words[activeWord];
        for (size_t i = 0; i < w.answer.size(); ++i) {
            int r = w.horizontal ? w.row : w.row + (int)i;
            int c = w.horizontal ? w.col + (int)i : w.col;
            GridCell* cell = getCell(r, c);
            if (cell && !cell->locked && cell->entered == '_') {
                cell->entered = (char)std::toupper((unsigned char)ch);
                cell->trayOrbIdx = orbIdx;
                if (orbIdx >= 0 && orbIdx < (int)trayOrbs.size()) {
                    trayOrbs[orbIdx].used = true;
                }
                int cellIdx = (int)(cell - &cells[0]);
                typedCellHistory.push_back(cellIdx);
                gAudio.playPop();
                checkWordCompletion();
                return;
            }
        }
    }

    void undoLastLetter() {
        if (typedCellHistory.empty()) return;
        int cellIdx = typedCellHistory.back();
        typedCellHistory.pop_back();
        if (cellIdx >= 0 && cellIdx < (int)cells.size()) {
            GridCell& cell = cells[cellIdx];
            if (!cell.locked) {
                if (cell.trayOrbIdx >= 0 && cell.trayOrbIdx < (int)trayOrbs.size()) {
                    trayOrbs[cell.trayOrbIdx].used = false;
                }
                cell.entered = '_';
                cell.trayOrbIdx = -1;
                gAudio.playClick();
            }
        }
    }

    void revealHint() {
        if (words.empty()) return;
        const auto& w = words[activeWord];
        for (size_t i = 0; i < w.answer.size(); ++i) {
            int r = w.horizontal ? w.row : w.row + (int)i;
            int c = w.horizontal ? w.col + (int)i : w.col;
            GridCell* cell = getCell(r, c);
            if (cell && (!cell->locked || cell->entered != cell->answer)) {
                if (cell->trayOrbIdx >= 0 && cell->trayOrbIdx < (int)trayOrbs.size()) {
                    trayOrbs[cell->trayOrbIdx].used = false;
                }
                cell->entered = cell->answer;
                cell->locked = true;
                cell->trayOrbIdx = -1;
                for (auto& orb : trayOrbs) {
                    if (!orb.used && !orb.letter.empty() && orb.letter[0] == cell->answer) {
                        orb.used = true;
                        break;
                    }
                }
                gAudio.playPop();
                checkWordCompletion();
                return;
            }
        }
    }

    Rectangle cellScreenRect(int r, int c, float& outTileSz) const {
        int cols = std::max(1, maxCol - minCol + 1);
        int rows = std::max(1, maxRow - minRow + 1);
        float gap = 3.0f;
        float maxW = 350.0f;
        float maxH = 360.0f;
        outTileSz = std::min({ 52.0f, (maxW - (cols - 1) * gap) / (float)cols, (maxH - (rows - 1) * gap) / (float)rows });
        float totalW = cols * (outTileSz + gap) - gap;
        float totalH = rows * (outTileSz + gap) - gap;
        float startX = 290.0f - totalW * 0.5f;
        float startY = 365.0f - totalH * 0.5f;

        float x = startX + (c - minCol) * (outTileSz + gap);
        float y = startY + (r - minRow) * (outTileSz + gap);
        return Rectangle{ x, y, outTileSz, outTileSz };
    }

    int update(float dt) {
        if (wrongFlashTimer > 0.0f) {
            wrongFlashTimer = std::max(0.0f, wrongFlashTimer - dt);
        }

        if (victoryModal.active) {
            victoryModal.update(dt);
            if (victoryModal.nextClicked) {
                setLevel((levelIndex + 1) % std::max(1, gGameData.levelCount()));
                return -1;
            }
            if (victoryModal.mapClicked) {
                victoryModal.hide();
                return Nav::SCREEN_MAP;
            }
            return -1;
        }

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) {
            undoLastLetter();
        }
        int key = GetCharPressed();
        while (key > 0) {
            if (std::isalpha(key)) {
                char upper = (char)std::toupper(key);
                int matchOrb = -1;
                for (size_t i = 0; i < trayOrbs.size(); ++i) {
                    if (!trayOrbs[i].used && !trayOrbs[i].letter.empty() && trayOrbs[i].letter[0] == upper) {
                        matchOrb = (int)i;
                        break;
                    }
                }
                typeLetter(upper, matchOrb);
            }
            key = GetCharPressed();
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = Input::Mouse();
            // Back / Settings Wood Gear button
            if (CheckCollisionPointRec(m, Rectangle{ 325, 8, 68, 68 })) {
                gAudio.playClick();
                return Nav::SCREEN_MAP;
            }
            // Hint button below Detective Fox or Reveal Lightbulb button
            if (CheckCollisionPointCircle(m, { 64, 598 }, 42.0f) ||
                CheckCollisionPointCircle(m, { 484, 754 }, 42.0f)) {
                revealHint();
                return -1;
            }
            // Clicking any typed (unlocked) cell in the crossword grid removes it (Undo!)
            float tileSz = 48.0f;
            for (size_t ci = 0; ci < cells.size(); ++ci) {
                auto& cell = cells[ci];
                Rectangle cRec = cellScreenRect(cell.row, cell.col, tileSz);
                if (!cell.locked && cell.entered != '_' && CheckCollisionPointRec(m, cRec)) {
                    if (cell.trayOrbIdx >= 0 && cell.trayOrbIdx < (int)trayOrbs.size()) {
                        trayOrbs[cell.trayOrbIdx].used = false;
                    }
                    cell.entered = '_';
                    cell.trayOrbIdx = -1;
                    typedCellHistory.erase(std::remove(typedCellHistory.begin(), typedCellHistory.end(), (int)ci), typedCellHistory.end());
                    gAudio.playClick();
                    return -1;
                }
            }
            // Tray Orbs (generous 30px click radius)
            for (size_t i = 0; i < trayOrbs.size(); ++i) {
                if (!trayOrbs[i].used && CheckCollisionPointCircle(m, trayOrbs[i].pos, 30.0f)) {
                    typeLetter(trayOrbs[i].letter[0], (int)i);
                    break;
                }
            }
        }
        return -1;
    }

    void draw(int w, int h) {
        float t = Theme::timeNow;
        Vector2 m = Input::Mouse();

        if (!UIAssets::drawStretch("crossword/bg_crossword_clean.png", Rectangle{ 0, 0, (float)w, (float)h })) {
            DrawRectangleGradientV(0, 0, w, h, Color{ 189, 226, 248, 255 }, Color{ 218, 240, 252, 255 });
        }

        float tileSz = 48.0f;
        if (!words.empty()) {
            const auto& activeW = words[activeWord];
            if (!activeW.answer.empty()) {
                Rectangle firstR = cellScreenRect(activeW.row, activeW.col, tileSz);
                int lastRow = activeW.horizontal ? activeW.row : activeW.row + (int)activeW.answer.size() - 1;
                int lastCol = activeW.horizontal ? activeW.col + (int)activeW.answer.size() - 1 : activeW.col;
                Rectangle lastR = cellScreenRect(lastRow, lastCol, tileSz);

                Rectangle glowRec{
                    firstR.x - 7.0f,
                    firstR.y - 7.0f,
                    (lastR.x + lastR.width) - firstR.x + 14.0f,
                    (lastR.y + lastR.height) - firstR.y + 14.0f
                };
                Color glowFill = (wrongFlashTimer > 0.0f) ? Color{ 239, 68, 68, 95 } : Color{ 253, 224, 71, 65 };
                Color glowRim  = (wrongFlashTimer > 0.0f) ? Color{ 220, 38, 38, 255 } : Color{ 250, 204, 21, 255 };
                UI::GradientRound(glowRec, 14.0f, glowFill, glowFill);
                UI::Rim(glowRec, 14.0f, glowRim, WHITE, Color{ 180, 83, 9, 245 });
            }
        }

        for (const auto& cell : cells) {
            Rectangle cellRec = cellScreenRect(cell.row, cell.col, tileSz);
            UI::DropShadow(cellRec, 10.0f, 3.0f, 0.30f, 2.0f);
            if (cell.entered != '_') {
                char ch = (char)std::toupper((unsigned char)cell.entered);
                std::string key = std::string("crossword/wood_tiles/wood_tile_") + ch + ".png";
                UIAssets::drawFit(key, cellRec);
            } else {
                UIAssets::drawFit("crossword/wood_tiles/wood_slot_recessed_empty.png", cellRec);
            }
        }

        Rectangle foxRec{ 4, 405, 154, 185 };
        UIAssets::drawFrameSequence("crossword/fox_rgba/frame_", 16, t, 12.0f, foxRec);
        float hintScale = CheckCollisionPointCircle(m, { 64, 598 }, 40.0f) ? 84.0f : 78.0f;
        UIAssets::drawCentered("crossword/btn_hint_magnifying_glass.png", { 64, 598 }, hintScale, hintScale);

        std::string defText = gGameData.levels.empty()
            ? "The practice of offering money for illegal favor"
            : gGameData.levels[levelIndex].word.definition_en;
        Theme::text3D("DEFINITION (Click empty slots via letters below / click tile to undo):", { 250, 664 },
                      12, Color{ 253, 224, 71, 255 }, Color{ 66, 32, 6, 255 }, 2.0f, BLACK, true, true);
        UIAssets::drawWrappedText3D(defText, Rectangle{ 34, 682, 428, 80 }, 17, WHITE, Color{ 15, 23, 42, 255 }, 2.0f, true);

        float revScale = CheckCollisionPointCircle(m, { 484, 754 }, 40.0f) ? 82.0f : 76.0f;
        UIAssets::drawCentered("crossword/btn_reveal_lightbulb.png", { 484, 754 }, revScale, revScale);

        const char* colorSuffixes[8] = {
            "_yellow.png", "_green.png", "_orange.png", "_red.png",
            "_teal.png", "_blue.png", "_purple.png", "_silver.png"
        };
        for (size_t i = 0; i < trayOrbs.size(); ++i) {
            const auto& orb = trayOrbs[i];
            if (!orb.used && !orb.letter.empty()) {
                char c = (char)std::toupper((unsigned char)orb.letter[0]);
                int colorIdx = (c - 'A') % 8;
                std::string tokKey = std::string("crossword/tray_tokens/token_") + c + colorSuffixes[colorIdx];
                bool hov = CheckCollisionPointCircle(m, orb.pos, 28.0f);
                float sz = hov ? 57.0f : 52.0f;
                UIAssets::drawCentered(tokKey, orb.pos, sz, sz);
            }
        }

        if (victoryModal.active) {
            victoryModal.draw(w, h);
        }
    }
};
