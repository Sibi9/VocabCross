#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

namespace Theme {
    extern int coins;
}

using json = nlohmann::json;

struct WordData {
    std::string word;
    std::string translation_ar;
    std::string definition_en;
    std::string synonym_en;
    std::string frequency;
    int difficulty = 1;
    std::string mode; // "wheel", "crossword", "image"
    std::vector<std::string> letters;
    std::string image_keyword;
};

struct UnitData {
    std::string id;
    std::string title_en;
    std::string title_ar;
    std::string island_name;
    std::vector<WordData> words;
};

struct LevelInfo {
    int global_index = 0;
    int unit_index = 0;
    int unit_level_index = 0;
    int word_index = 0;
    WordData data;
    bool unlocked = false;
    int stars = 0;
    int score = 0;

    // Compatibility fields:
    WordData word;
    int unit = 0;
    std::string mode;
};

class GameDataManager {
public:
    std::vector<UnitData> units;
    std::vector<LevelInfo> levels;
    int current_level = 0;
    int total_stars = 0;
    int streak_day = 4;
    int player_coins = 450;

    bool loadFromFile(const std::string& json_path) {
        return loadData(json_path);
    }

    bool loadData(const std::string& json_path) {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << json_path << std::endl;
            return false;
        }
        try {
            json j;
            file >> j;
            units.clear();
            levels.clear();

            int global_idx = 0;
            for (size_t u_idx = 0; u_idx < j["units"].size(); ++u_idx) {
                const auto& u = j["units"][u_idx];
                UnitData unit;
                unit.id = u["id"].get<std::string>();
                unit.title_en = u["title_en"].get<std::string>();
                unit.title_ar = u["title_ar"].get<std::string>();
                unit.island_name = u["island_name"].get<std::string>();

                for (size_t w_idx = 0; w_idx < u["words"].size(); ++w_idx) {
                    const auto& w = u["words"][w_idx];
                    WordData word;
                    word.word = w["word"].get<std::string>();
                    word.translation_ar = w["translation_ar"].get<std::string>();
                    word.definition_en = w["definition_en"].get<std::string>();
                    word.synonym_en = w["synonym_en"].get<std::string>();
                    word.frequency = w["frequency"].get<std::string>();
                    word.difficulty = w["difficulty"].get<int>();
                    word.mode = w["mode"].get<std::string>();
                    word.image_keyword = w["image_keyword"].get<std::string>();
                    for (const auto& l : w["letters"]) word.letters.push_back(l.get<std::string>());
                    if (word.letters.empty()) {
                        for (char c : word.word) word.letters.push_back(std::string(1, c));
                    }
                    unit.words.push_back(word);

                    LevelInfo lvl;
                    lvl.global_index = global_idx;
                    lvl.unit_index = static_cast<int>(u_idx);
                    lvl.unit_level_index = static_cast<int>(w_idx);
                    lvl.word_index = static_cast<int>(w_idx);
                    lvl.data = word;
                    lvl.word = word;
                    lvl.unit = static_cast<int>(u_idx);
                    lvl.mode = word.mode;
                    lvl.unlocked = (global_idx == 0);
                    levels.push_back(lvl);
                    ++global_idx;
                }
                units.push_back(unit);
            }

            current_level = 0;
            // Every island starts playable; progress inside each island is sequential.
            for (size_t u = 0; u < units.size(); ++u) {
                int start = unitStart(static_cast<int>(u));
                if (start >= 0 && start < levelCount()) levels[start].unlocked = true;
            }
            recalcTotalStars();
            std::cout << "Loaded " << units.size() << " units and " << levels.size()
                      << " levels successfully." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JSON Parse error: " << e.what() << std::endl;
            return false;
        }
    }

    int levelCount() const { return static_cast<int>(levels.size()); }

    int unitStart(int unit) const {
        for (int i = 0; i < levelCount(); ++i) {
            if (levels[i].unit_index == unit) return i;
        }
        return 0;
    }

    int unitLevelCount(int unit) const {
        int n = 0;
        for (const auto& l : levels) n += (l.unit_index == unit ? 1 : 0);
        return n;
    }

    int unitCount(int unit) const {
        return unitLevelCount(unit);
    }

    int unitOfLevel(int index) const {
        if (index < 0 || index >= levelCount()) return 0;
        return levels[index].unit_index;
    }

    int indexOfWord(const std::string& word) const {
        for (int i = 0; i < levelCount(); ++i) {
            if (levels[i].data.word == word) return i;
        }
        return -1;
    }

    int firstLevelOfMode(int unit, const std::string& mode) const {
        for (int i = 0; i < levelCount(); ++i) {
            if (levels[i].unit_index == unit && levels[i].data.mode == mode) return i;
        }
        return -1;
    }

    void unlockThrough(int index) {
        for (int i = 0; i <= index && i < levelCount(); ++i) levels[i].unlocked = true;
    }

    bool saveProfile(const std::string& path = "save_profile.json") {
        try {
            json j;
            j["version"] = 1;
            j["coins"] = Theme::coins;
            j["streak_day"] = streak_day;
            j["current_level"] = current_level;
            j["total_stars"] = total_stars;

            json levelsArray = json::array();
            for (const auto& l : levels) {
                json lj;
                lj["id"] = l.global_index;
                lj["word"] = l.data.word;
                lj["stars"] = l.stars;
                lj["unlocked"] = l.unlocked;
                levelsArray.push_back(lj);
            }
            j["levels"] = levelsArray;

            std::string tmpPath = path + ".tmp";
            {
                std::ofstream out(tmpPath);
                if (!out.is_open()) return false;
                out << j.dump(2);
            }
            std::error_code ec;
            std::filesystem::rename(tmpPath, path, ec);
            if (ec) {
                std::filesystem::copy_file(tmpPath, path, std::filesystem::copy_options::overwrite_existing, ec);
                std::filesystem::remove(tmpPath, ec);
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    bool loadProfile(const std::string& path = "save_profile.json") {
        try {
            std::error_code ec;
            if (!std::filesystem::exists(path, ec) || ec) return false;
            std::ifstream in(path);
            if (!in.is_open()) return false;
            json j;
            in >> j;

            if (j.contains("coins") && j["coins"].is_number_integer()) {
                Theme::coins = j["coins"].get<int>();
                player_coins = Theme::coins;
            }
            if (j.contains("streak_day") && j["streak_day"].is_number_integer()) {
                streak_day = j["streak_day"].get<int>();
            }
            if (j.contains("current_level") && j["current_level"].is_number_integer()) {
                current_level = j["current_level"].get<int>();
            }
            if (j.contains("levels") && j["levels"].is_array()) {
                for (const auto& lj : j["levels"]) {
                    if (lj.contains("id") && lj["id"].is_number_integer()) {
                        int id = lj["id"].get<int>();
                        if (id >= 0 && id < levelCount()) {
                            if (lj.contains("stars")) levels[id].stars = lj["stars"].get<int>();
                            if (lj.contains("unlocked")) levels[id].unlocked = lj["unlocked"].get<bool>();
                        }
                    }
                }
            }
            recalcTotalStars();
            return true;
        } catch (...) {
            return false;
        }
    }

    void completeLevel(int index, int stars_earned) {
        if (index < 0 || index >= levelCount()) return;
        levels[index].stars = std::max(levels[index].stars, stars_earned);
        levels[index].unlocked = true;
        if (index + 1 < levelCount()) levels[index + 1].unlocked = true;
        recalcTotalStars();
        saveProfile();
    }

    void recalcTotalStars() {
        total_stars = 0;
        for (const auto& l : levels) total_stars += l.stars;
    }

    void resetProgress() {
        for (int i = 0; i < levelCount(); ++i) {
            levels[i].stars = 0;
            levels[i].score = 0;
            levels[i].unlocked = false;
        }
        for (int u = 0; u < (int)units.size(); ++u) {
            int start = unitStart(u);
            if (start >= 0 && start < levelCount()) levels[start].unlocked = true;
        }
        current_level = 0;
        total_stars = 0;
        saveProfile();
    }

    std::string unitShortTitle(int unit) const {
        static const char* names[] = { "ETHICS", "SAFETY", "ASTRONOMY", "FEELINGS" };
        if (unit < 0 || unit > 3) return "VOCAB";
        return names[unit];
    }

    std::string unitQuestTitle(int unit) const {
        static const char* names[] = { "ETHICS QUEST", "SAFETY QUEST", "ASTRONOMY QUEST", "FEELINGS QUEST" };
        if (unit < 0 || unit > 3) return "VOCAB QUEST";
        return names[unit];
    }

    std::string islandTitle(int unit) const {
        static const char* names[] = { "ETHICS ISLAND", "SAFETY ISLAND", "SPACE ISLAND", "EMOTIONS ISLAND" };
        if (unit < 0 || unit > 3) return "VOCAB ISLAND";
        return names[unit];
    }

    std::string macronFrequencyEn(const std::string& freq) const {
        if (freq == "high") return "Bac Frequency: Common";
        if (freq == "advanced") return "Bac Frequency: Advanced";
        return "Bac Frequency: Core";
    }

    std::string macronFrequencyAr(const std::string& freq) const {
        if (freq == "high") return "متداول بكثرة في البكالوريا";
        if (freq == "advanced") return "مصطلح متقدم في البكالوريا";
        return "من أساسيات البكالوريا";
    }
};

inline GameDataManager gGameData;

// ---------------------------------------------------------------------------
// Lightweight navigation helpers shared by every stage and by main.cpp.
// ---------------------------------------------------------------------------
namespace Nav {
    enum ScreenId {
        SCREEN_HOME = 0,
        SCREEN_MAP = 1,
        SCREEN_CROSSWORD = 2,
        SCREEN_IMAGE_GUESS = 3,
        SCREEN_WHEEL = 4,
        SCREEN_SHOP = 5,
        SCREEN_STREAK = 6
    };

    inline int stageScreenForMode(const std::string& mode) {
        if (mode == "crossword") return SCREEN_CROSSWORD;
        if (mode == "image" || mode == "image_guess") return SCREEN_IMAGE_GUESS;
        if (mode == "wheel") return SCREEN_WHEEL;
        return SCREEN_CROSSWORD;
    }

    inline int stageScreenForLevel(int level) {
        if (level < 0 || level >= gGameData.levelCount()) return SCREEN_MAP;
        std::string m = gGameData.levels[level].mode;
        if (m.empty()) m = gGameData.levels[level].data.mode;
        return stageScreenForMode(m);
    }
}
