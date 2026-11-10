#pragma once
#include "game_data.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// A small, deterministic crossword layout engine shared by the Crossword and
// Wheel stages.  Every cell belongs to at least one real word; there are no
// decorative "dead" tiles.  A level contains the target word plus one or two
// companion words from the same BAC unit (or a generic fallback word).
// ---------------------------------------------------------------------------
struct PuzzleWord {
    WordData data;
    std::string answer;
    int row = 0;
    int col = 0;
    bool horizontal = true;
    bool solved = false;
    bool wrong = false;
};

struct PuzzleCell {
    int row = 0;
    int col = 0;
    char answer = '\0';
    char entered = '_';
    bool locked = false;
    std::vector<int> words;
};

class CrosswordPuzzle {
public:
    std::vector<PuzzleWord> words;
    std::vector<PuzzleCell> cells;
    int minRow = 0, maxRow = 0, minCol = 0, maxCol = 0;
    int activeWord = 0;

    void clear() {
        words.clear();
        cells.clear();
        minRow = maxRow = minCol = maxCol = 0;
        activeWord = 0;
    }

    static WordData fallbackWord(const std::string& w) {
        WordData d;
        d.word = w;
        d.translation_ar = "كلمة إضافية";
        d.definition_en = "A bonus crossword word";
        d.synonym_en = "Bonus";
        d.frequency = "high";
        d.mode = "crossword";
        d.letters.clear();
        for (char c : w) d.letters.push_back(std::string(1, c));
        d.image_keyword = "";
        return d;
    }

    static int sharedLetters(const std::string& a, const std::string& b) {
        int n = 0;
        std::string used;
        for (char c : a) {
            if (used.find(c) != std::string::npos) continue;
            if (b.find(c) != std::string::npos) { ++n; used += c; }
        }
        return n;
    }

    bool canPlace(const std::map<std::pair<int, int>, PuzzleCell>& grid,
                  const std::string& answer, int row, int col, bool horizontal,
                  int& intersections) const {
        intersections = 0;
        int len = (int)answer.size();
        if (len < 2) return false;
        if (horizontal && (col < -14 || col + len > 14)) return false;
        if (!horizontal && (row < -14 || row + len > 14)) return false;
        if (row < -14 || row > 14) return false;
        if (col < -14 || col > 14) return false;

        for (int i = 0; i < len; ++i) {
            int r = row + (horizontal ? 0 : i);
            int c = col + (horizontal ? i : 0);
            auto it = grid.find({ r, c });
            if (it != grid.end()) {
                if (it->second.answer != answer[i]) return false;
                ++intersections;
                continue;
            }
            // A brand-new cell must not touch an existing perpendicular word
            // (this keeps the crossword clean and readable).
            if (horizontal) {
                if (grid.count({ r - 1, c }) || grid.count({ r + 1, c })) return false;
            } else {
                if (grid.count({ r, c - 1 }) || grid.count({ r, c + 1 })) return false;
            }
        }
        // No extension at either end.
        if (horizontal) {
            if (grid.count({ row, col - 1 }) || grid.count({ row, col + len })) return false;
        } else {
            if (grid.count({ row - 1, col }) || grid.count({ row + len, col })) return false;
        }
        return intersections >= 1;
    }

    void place(const WordData& data, int row, int col, bool horizontal,
               std::map<std::pair<int, int>, PuzzleCell>& grid) {
        PuzzleWord pw;
        pw.data = data;
        pw.answer = data.word;
        pw.row = row;
        pw.col = col;
        pw.horizontal = horizontal;
        int wi = (int)words.size();
        words.push_back(pw);
        for (int i = 0; i < (int)pw.answer.size(); ++i) {
            int r = row + (horizontal ? 0 : i);
            int c = col + (horizontal ? i : 0);
            auto key = std::make_pair(r, c);
            auto it = grid.find(key);
            if (it == grid.end()) {
                PuzzleCell cell;
                cell.row = r;
                cell.col = c;
                cell.answer = pw.answer[i];
                cell.words.push_back(wi);
                grid[key] = cell;
            } else {
                it->second.words.push_back(wi);
            }
        }
    }

    bool build(const WordData& target, const UnitData& unit, int maxCompanions = 2) {
        clear();
        if (target.word.size() < 2) return false;

        std::map<std::pair<int, int>, PuzzleCell> grid;
        place(target, 0, 0, true, grid);

        std::vector<WordData> candidates;
        for (const auto& w : unit.words) {
            if (w.word == target.word) continue;
            if (w.word.size() < 3 || w.word.size() > 8) continue;
            candidates.push_back(w);
        }
        static const char* fallback[] = {
            "STAR", "MOON", "FIRE", "WATER", "HOUSE", "PLANT", "HAPPY", "MUSIC",
            "LIGHT", "DREAM", "EARTH", "OCEAN", "RIVER", "STONE", "CLOUD", "SMILE",
            "HEART", "MIND", "BRAIN", "WORLD", "PEACE", "TRUST", "HONEST", "FRIEND",
            "BOOK", "GREEN", "NIGHT", "SUN", "SKY", "ROAD", "BIRD", "GOLD"
        };
        for (const char* w : fallback) {
            std::string s(w);
            bool dup = false;
            for (const auto& c : candidates) if (c.word == s) { dup = true; break; }
            if (dup || s == target.word) continue;
            candidates.push_back(fallbackWord(s));
        }

        std::sort(candidates.begin(), candidates.end(), [&](const WordData& a, const WordData& b) {
            int sa = sharedLetters(a.word, target.word);
            int sb = sharedLetters(b.word, target.word);
            if (sa != sb) return sa > sb;
            if (a.word.size() != b.word.size()) return a.word.size() > b.word.size();
            return a.word < b.word;
        });

        std::vector<bool> used(candidates.size(), false);
        for (int pass = 0; pass < maxCompanions; ++pass) {
            int bestScore = -100000;
            int bestCandidate = -1;
            int bestRow = 0, bestCol = 0;
            bool bestHorizontal = true;
            for (size_t ci = 0; ci < candidates.size(); ++ci) {
                if (used[ci]) continue;
                const std::string& cw = candidates[ci].word;
                // Bound the final grid to a readable 9x9-ish crossword.
                for (size_t wi = 0; wi < words.size(); ++wi) {
                    const auto& existing = words[wi];
                    for (int ei = 0; ei < (int)existing.answer.size(); ++ei) {
                        char ec = existing.answer[ei];
                        int er = existing.row + (existing.horizontal ? 0 : ei);
                        int ecc = existing.col + (existing.horizontal ? ei : 0);
                        for (int k = 0; k < (int)cw.size(); ++k) {
                            if (cw[k] != ec) continue;
                            bool horizontal = !existing.horizontal;
                            int r = er - (horizontal ? 0 : k);
                            int c = ecc - (horizontal ? k : 0);
                            int inter = 0;
                            if (!canPlace(grid, cw, r, c, horizontal, inter)) continue;
                            int newCells = (int)cw.size() - inter;
                            if ((int)grid.size() + newCells > 18) continue;
                            int score = inter * 100 + (int)cw.size() * 3 - std::abs(r) - std::abs(c);
                            // Prefer words that are not too long for the tray.
                            if (cw.size() > 7) score -= 12;
                            if (score > bestScore) {
                                bestScore = score;
                                bestCandidate = (int)ci;
                                bestRow = r;
                                bestCol = c;
                                bestHorizontal = horizontal;
                            }
                        }
                    }
                }
            }
            if (bestCandidate < 0) break;
            place(candidates[bestCandidate], bestRow, bestCol, bestHorizontal, grid);
            used[bestCandidate] = true;
        }

        // Normalize coordinates and materialize cells.
        int minR = 999, maxR = -999, minC = 999, maxC = -999;
        for (const auto& kv : grid) {
            minR = std::min(minR, kv.first.first);
            maxR = std::max(maxR, kv.first.first);
            minC = std::min(minC, kv.first.second);
            maxC = std::max(maxC, kv.first.second);
        }
        minRow = minR; maxRow = maxR; minCol = minC; maxCol = maxC;
        for (auto& w : words) {
            w.row -= minR;
            w.col -= minC;
        }
        for (const auto& kv : grid) {
            PuzzleCell c = kv.second;
            c.row -= minR;
            c.col -= minC;
            cells.push_back(c);
        }
        std::sort(cells.begin(), cells.end(), [](const PuzzleCell& a, const PuzzleCell& b) {
            if (a.row != b.row) return a.row < b.row;
            return a.col < b.col;
        });
        activeWord = 0;
        return !cells.empty();
    }

    int cols() const { return maxCol - minCol + 1; }
    int rows() const { return maxRow - minRow + 1; }

    int cellIndex(int row, int col) const {
        for (size_t i = 0; i < cells.size(); ++i) {
            if (cells[i].row == row && cells[i].col == col) return (int)i;
        }
        return -1;
    }

    std::vector<int> wordCellIndices(int wi) const {
        std::vector<int> out;
        if (wi < 0 || wi >= (int)words.size()) return out;
        const auto& w = words[wi];
        for (int i = 0; i < (int)w.answer.size(); ++i) {
            int r = w.row + (w.horizontal ? 0 : i);
            int c = w.col + (w.horizontal ? i : 0);
            int ci = cellIndex(r, c);
            if (ci >= 0) out.push_back(ci);
        }
        return out;
    }

    std::string wordEntered(int wi) const {
        std::string s;
        for (int ci : wordCellIndices(wi)) s += cells[ci].entered;
        return s;
    }

    bool wordFilled(int wi) const {
        for (int ci : wordCellIndices(wi)) if (cells[ci].entered == '_') return false;
        return true;
    }

    bool wordCorrect(int wi) const {
        if (wi < 0 || wi >= (int)words.size()) return false;
        std::string e = wordEntered(wi);
        return e == words[wi].answer;
    }

    bool allSolved() const {
        for (const auto& w : words) if (!w.solved) return false;
        return !words.empty();
    }

    int firstEmptyCell(int wi) const {
        for (int ci : wordCellIndices(wi)) {
            if (cells[ci].entered == '_') return ci;
        }
        return -1;
    }

    int nextUnsolvedWord(int after) const {
        for (int step = 1; step <= (int)words.size(); ++step) {
            int wi = (after + step) % (int)words.size();
            if (!words[wi].solved) return wi;
        }
        return -1;
    }

    void lockWord(int wi) {
        for (int ci : wordCellIndices(wi)) cells[ci].locked = true;
    }

    void markSolvedWords() {
        for (int wi = 0; wi < (int)words.size(); ++wi) {
            if (words[wi].solved) continue;
            if (wordFilled(wi)) {
                words[wi].wrong = !wordCorrect(wi);
                if (!words[wi].wrong) {
                    words[wi].solved = true;
                    lockWord(wi);
                }
            } else {
                words[wi].wrong = false;
            }
        }
    }

    void solveWord(int wi) {
        if (wi < 0 || wi >= (int)words.size()) return;
        auto indices = wordCellIndices(wi);
        for (size_t i = 0; i < indices.size() && i < words[wi].answer.size(); ++i) {
            cells[indices[i]].entered = words[wi].answer[i];
        }
        words[wi].solved = true;
        words[wi].wrong = false;
        lockWord(wi);
    }

    void solveAll() {
        for (int wi = 0; wi < (int)words.size(); ++wi) solveWord(wi);
    }

    std::vector<char> supplyLetters() const {
        std::vector<char> out;
        for (const auto& c : cells) if (c.answer) out.push_back(c.answer);
        return out;
    }

    int firstWordAtCell(int cellIdx) const {
        if (cellIdx < 0 || cellIdx >= (int)cells.size()) return 0;
        if (cells[cellIdx].words.empty()) return 0;
        return cells[cellIdx].words[0];
    }
};

using CrosswordLayout = CrosswordPuzzle;
