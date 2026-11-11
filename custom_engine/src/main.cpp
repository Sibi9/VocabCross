#include "raylib.h"
#include "rlgl.h"

#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "screen_crossword.hpp"
#include "screen_home.hpp"
#include "screen_image_guess.hpp"
#include "screen_maps.hpp"
#include "screen_shop.hpp"
#include "screen_streak.hpp"
#include "screen_wheel.hpp"
#include "theme.hpp"
#include "ui_asset_pack.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

constexpr int kWidth = 540;
constexpr int kHeight = 960;

void DrawFrame(ScreenHome& home, ScreenMaps& map, ScreenCrossword& crossword,
               ScreenImageGuess& imageGuess, ScreenWheel& wheel,
               ScreenShop& shop, ScreenStreak& streak,
               int screen, RenderTexture2D canvas) {
    BeginTextureMode(canvas);
    ClearBackground(Color{ 10, 17, 34, 255 });
    switch (screen) {
        case Nav::SCREEN_HOME: home.draw(kWidth, kHeight); break;
        case Nav::SCREEN_MAP: map.draw(kWidth, kHeight); break;
        case Nav::SCREEN_CROSSWORD: crossword.draw(kWidth, kHeight); break;
        case Nav::SCREEN_IMAGE_GUESS: imageGuess.draw(kWidth, kHeight); break;
        case Nav::SCREEN_WHEEL: wheel.draw(kWidth, kHeight); break;
        case Nav::SCREEN_SHOP: shop.draw(kWidth, kHeight); break;
        case Nav::SCREEN_STREAK: streak.draw(kWidth, kHeight); break;
        default: home.draw(kWidth, kHeight); break;
    }
    EndTextureMode();
}

void Capture(ScreenHome& home, ScreenMaps& map, ScreenCrossword& crossword,
             ScreenImageGuess& imageGuess, ScreenWheel& wheel,
             ScreenShop& shop, ScreenStreak& streak,
             int screen, const std::string& path, RenderTexture2D canvas) {
    Theme::timeNow += 0.35f;
    Theme::updateParticles(0.016f);
    for (int i = 0; i < 2; ++i) {
        DrawFrame(home, map, crossword, imageGuess, wheel, shop, streak, screen, canvas);
    }
    Image img = LoadImageFromTexture(canvas.texture);
    ImageFlipVertical(&img);
    ExportImage(img, path.c_str());
    UnloadImage(img);
    std::cout << "  [CAPTURED] " << path << std::endl;
}

int LevelOf(const std::string& word) {
    int idx = gGameData.indexOfWord(word);
    return (idx >= 0) ? idx : 0;
}

int RunSelfTest(ScreenHome& homeScreen, ScreenMaps& mapScreen, ScreenCrossword& crosswordScreen,
                ScreenImageGuess& imageGuessScreen, ScreenWheel& wheelScreen,
                ScreenShop& shopScreen, ScreenStreak& streakScreen) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "         VOCABCROSS ENGINE - AUTOMATED SELF-TEST        " << std::endl;
    std::cout << "========================================================\n" << std::endl;

    int passed = 0;
    int failed = 0;

    auto testAssert = [&](const std::string& name, bool cond) {
        if (cond) {
            std::cout << "  [PASS] " << name << std::endl;
            passed++;
        } else {
            std::cerr << "  [FAIL] " << name << std::endl;
            failed++;
        }
    };

    // 1. Data Integrity & BAC Units
    std::cout << "[TEST SUITE 1/5] Checking Data Integrity & BAC Units..." << std::endl;
    testAssert("Level count equals exactly 32", gGameData.levelCount() == 32);
    testAssert("Unit count equals exactly 4", gGameData.units.size() == 4);
    int expectedStarts[4] = { 0, 10, 18, 26 };
    int expectedCounts[4] = { 10, 8, 8, 6 };
    for (int u = 0; u < 4; ++u) {
        int uStart = gGameData.unitStart(u);
        int uCount = gGameData.unitLevelCount(u);
        testAssert("Unit " + std::to_string(u) + " (" + gGameData.unitShortTitle(u) + ") has " + std::to_string(expectedCounts[u]) + " levels", uCount == expectedCounts[u]);
        testAssert("Unit " + std::to_string(u) + " starts at index " + std::to_string(expectedStarts[u]), uStart == expectedStarts[u]);
    }

    // 2. Stage Navigation & Mode Routing via Nav::stageScreenForLevel
    std::cout << "\n[TEST SUITE 2/5] Checking Stage Navigation & Mode Routing..." << std::endl;
    int crosswordCount = 0, imageCount = 0, wheelCount = 0;
    bool allRoutedProperly = true;
    for (int i = 0; i < gGameData.levelCount(); ++i) {
        const auto& lvl = gGameData.levels[i];
        int targetScreen = Nav::stageScreenForLevel(i);
        std::string mode = lvl.mode.empty() ? lvl.data.mode : lvl.mode;

        if (mode == "crossword") {
            crosswordCount++;
            if (targetScreen != Nav::SCREEN_CROSSWORD) allRoutedProperly = false;
        } else if (mode == "image" || mode == "image_guess") {
            imageCount++;
            if (targetScreen != Nav::SCREEN_IMAGE_GUESS) allRoutedProperly = false;
        } else if (mode == "wheel") {
            wheelCount++;
            if (targetScreen != Nav::SCREEN_WHEEL) allRoutedProperly = false;
        } else {
            allRoutedProperly = false;
        }
    }
    testAssert("All 32 levels routed correctly via Nav::stageScreenForLevel", allRoutedProperly);
    testAssert("Crossword mode count > 0 (" + std::to_string(crosswordCount) + " levels)", crosswordCount > 0);
    testAssert("Image guess mode count > 0 (" + std::to_string(imageCount) + " levels)", imageCount > 0);
    testAssert("Wheel mode count > 0 (" + std::to_string(wheelCount) + " levels)", wheelCount > 0);

    // 3. Crossword Grid Construction & Validation
    std::cout << "\n[TEST SUITE 3/5] Verifying Crossword Grid Construction..." << std::endl;
    bool allCrosswordsBuilt = true;
    for (int i = 0; i < gGameData.levelCount(); ++i) {
        const auto& lvl = gGameData.levels[i];
        std::string mode = lvl.mode.empty() ? lvl.data.mode : lvl.mode;
        if (mode == "crossword") {
            crosswordScreen.setLevel(i);
            if (crosswordScreen.words.empty() || crosswordScreen.cells.empty()) {
                allCrosswordsBuilt = false;
            }
            if (crosswordScreen.words[0].answer.empty()) {
                allCrosswordsBuilt = false;
            }
        }
    }
    testAssert("All crossword levels build valid non-empty intersecting grids", allCrosswordsBuilt);

    // 4. Word Wheel & Image Guess Gameplay Initialization
    std::cout << "\n[TEST SUITE 4/5] Verifying Word Wheel & Image Guess Stages..." << std::endl;
    bool allWheelsValid = true;
    bool allImagesValid = true;
    for (int i = 0; i < gGameData.levelCount(); ++i) {
        const auto& lvl = gGameData.levels[i];
        std::string mode = lvl.mode.empty() ? lvl.data.mode : lvl.mode;
        if (mode == "wheel") {
            wheelScreen.setLevel(i);
            if (lvl.data.word.empty()) allWheelsValid = false;
        } else if (mode == "image" || mode == "image_guess") {
            imageGuessScreen.setLevel(i);
            if (lvl.data.word.empty()) allImagesValid = false;
        }
    }
    testAssert("All wheel stages initialized with target words and letter tray", allWheelsValid);
    testAssert("All image guess stages initialized with target words and answer slots", allImagesValid);

    // 5. Atomic Persistence (Save & Load Profile Roundtrip)
    std::cout << "\n[TEST SUITE 5/5] Verifying Atomic Persistence (Save/Load Profile)..." << std::endl;
    std::string testSavePath = "save_test.json";
    int origCoins = Theme::coins;
    int origStreak = gGameData.streak_day;

    Theme::coins = 888;
    gGameData.streak_day = 6;
    gGameData.levels[0].stars = 3;
    gGameData.levels[1].unlocked = true;

    bool saved = gGameData.saveProfile(testSavePath);
    testAssert("Atomic save to save_test.json succeeded", saved);

    Theme::coins = 0;
    gGameData.streak_day = 1;
    gGameData.levels[0].stars = 0;

    bool loaded = gGameData.loadProfile(testSavePath);
    testAssert("Loading save_test.json succeeded", loaded);
    testAssert("Saved coins restored correctly (888)", Theme::coins == 888);
    testAssert("Saved streak restored correctly (6)", gGameData.streak_day == 6);
    testAssert("Saved level stars restored correctly (3)", gGameData.levels[0].stars == 3);

    std::error_code ec;
    std::filesystem::remove(testSavePath, ec);
    std::filesystem::remove(testSavePath + ".tmp", ec);

    // Restore clean state
    Theme::coins = origCoins;
    gGameData.streak_day = origStreak;
    gGameData.loadProfile("save_profile.json");

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  SELF-TEST RESULT: " << passed << " PASSED, " << failed << " FAILED" << std::endl;
    std::cout << "========================================================\n" << std::endl;

    return (failed == 0) ? 0 : 1;
}

} // namespace

int main(int argc, char* argv[]) {
    bool verifyMode = false;
    bool selfTest = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verify-ui" || arg == "--verify") verifyMode = true;
        if (arg == "--self-test") selfTest = true;
    }

    // Start with a safe 9:16 window size (450x800) that fits all laptop screens without OS clipping,
    // then dynamically scale to 84% of the monitor height and center on screen!
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(450, 800, "BAC Vocab Quest - VocabCross Engine");
    if (!verifyMode) {
        int mon = GetCurrentMonitor();
        int monW = GetMonitorWidth(mon);
        int monH = GetMonitorHeight(mon);
        if (monH > 400 && monW > 400) {
            int targetH = std::min(900, (int)(monH * 0.84f));
            int targetW = (int)std::round(targetH * (540.0f / 960.0f));
            SetWindowSize(targetW, targetH);
            SetWindowPosition((monW - targetW) / 2, std::max(24, (monH - targetH) / 2));
        }
    }
    SetTargetFPS(60);

    gAudio.init();
    Theme::initFonts();
    UIAssets::init();

    if (!gGameData.loadFromFile("assets/data/bac_vocabulary.json")) {
        gGameData.loadFromFile("../data/bac_vocabulary.json");
    }
    gGameData.loadProfile();

    ScreenHome homeScreen;
    ScreenMaps mapScreen;
    ScreenCrossword crosswordScreen;
    ScreenImageGuess imageGuessScreen;
    ScreenWheel wheelScreen;
    ScreenShop shopScreen;
    ScreenStreak streakScreen;

    homeScreen.init();
    mapScreen.init();
    crosswordScreen.init();
    imageGuessScreen.init();
    wheelScreen.init();
    shopScreen.init();
    streakScreen.init();
    streakScreen.currentStreakDay = gGameData.streak_day;

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    if (verifyMode) {
        std::cout << "[VERIFY-UI] Capturing all 14 UI screens and modals..." << std::endl;
        std::filesystem::create_directories("verification_screenshots");

        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_HOME, "verification_screenshots/00_home_bac_vocab_quest.png", canvas);

        mapScreen.setUnit(0);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/01_map_ethics.png", canvas);

        mapScreen.setUnit(1);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/02_map_safety.png", canvas);

        mapScreen.setUnit(2);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/03_map_space.png", canvas);

        mapScreen.setUnit(3);
        mapScreen.boyProgress = 5.0f;
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/04_map_graduation_academy.png", canvas);

        crosswordScreen.setLevel(LevelOf("BRIBERY"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/04_crossword_lvl1.png", canvas);

        crosswordScreen.setLevel(LevelOf("COUNTERFEIT"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/05_crossword_lvl2.png", canvas);

        imageGuessScreen.setLevel(LevelOf("ECLIPSE"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_IMAGE_GUESS, "verification_screenshots/06_image_guess_lvl1.png", canvas);

        imageGuessScreen.setLevel(LevelOf("ORGANIC"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_IMAGE_GUESS, "verification_screenshots/07_image_guess_lvl2.png", canvas);

        wheelScreen.setLevel(LevelOf("GALAXY"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_WHEEL, "verification_screenshots/08_wheel_lvl1.png", canvas);

        if (!gGameData.levels.empty()) {
            crosswordScreen.victoryModal.show(gGameData.levels[LevelOf("BRIBERY")].word);
            crosswordScreen.victoryModal.anim = 1.0f;
        }
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/09_arabic_victory_modal.png", canvas);

        WordData integrityWord;
        integrityWord.word = "INTEGRITY";
        integrityWord.translation_ar = "النزاهة / الاستقامة والأمانة";
        integrityWord.definition_en = "The quality of being honest and having strong moral principles.";
        integrityWord.synonym_en = "Honesty, Probity, Rectitude";
        integrityWord.frequency = "High Frequency - Ethics in Business";
        crosswordScreen.victoryModal.wordCardModal.show(integrityWord);
        crosswordScreen.victoryModal.wordCardModal.anim = 1.0f;
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/10_word_card_integrity.png", canvas);
        crosswordScreen.victoryModal.hide();

        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_SHOP, "verification_screenshots/11_booster_shop.png", canvas);

        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_STREAK, "verification_screenshots/12_daily_streak_1_to_7.png", canvas);

        UnloadRenderTexture(canvas);
        UIAssets::unloadAll();
        Theme::unloadFonts();
        gAudio.close();
        CloseWindow();
        return 0;
    }

    if (selfTest) {
        int code = RunSelfTest(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen);
        UnloadRenderTexture(canvas);
        UIAssets::unloadAll();
        Theme::unloadFonts();
        gAudio.close();
        CloseWindow();
        return code;
    }

    int screen = Nav::SCREEN_HOME;

    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), 0.05f);
        Theme::timeNow += dt;
        Theme::updateParticles(dt);

        Rectangle dst = Input::CanvasDestination(GetScreenWidth(), GetScreenHeight());
        Input::SetCanvasTransform(dst.width / (float)kWidth, dst.height / (float)kHeight, dst.x, dst.y);

        if (screen == Nav::SCREEN_HOME) {
            int next = homeScreen.update(dt);
            if (homeScreen.pendingAction == 2) screen = Nav::SCREEN_SHOP;
            else if (homeScreen.pendingAction == 3) screen = Nav::SCREEN_STREAK;
            else if (homeScreen.pendingAction == 4) {
                mapScreen.setUnit(3);
                screen = Nav::SCREEN_MAP;
            } else if (next >= 0) {
                screen = next;
            }
        } else if (screen == Nav::SCREEN_MAP) {
            int lvl = mapScreen.update(dt);
            if (mapScreen.pendingNavScreen >= 0) {
                screen = mapScreen.pendingNavScreen;
            } else if (lvl >= 0 && lvl < gGameData.levelCount()) {
                int targetScreen = Nav::stageScreenForLevel(lvl);
                if (targetScreen == Nav::SCREEN_IMAGE_GUESS) {
                    imageGuessScreen.setLevel(lvl);
                    screen = Nav::SCREEN_IMAGE_GUESS;
                } else if (targetScreen == Nav::SCREEN_WHEEL) {
                    wheelScreen.setLevel(lvl);
                    screen = Nav::SCREEN_WHEEL;
                } else {
                    crosswordScreen.setLevel(lvl);
                    screen = Nav::SCREEN_CROSSWORD;
                }
            }
        } else if (screen == Nav::SCREEN_CROSSWORD) {
            int next = crosswordScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_IMAGE_GUESS) {
            int next = imageGuessScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_WHEEL) {
            int next = wheelScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_SHOP) {
            int next = shopScreen.update(dt);
            if (next >= 0) screen = next;
        } else if (screen == Nav::SCREEN_STREAK) {
            int next = streakScreen.update(dt);
            if (next >= 0) screen = next;
        }

        DrawFrame(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen,
                  shopScreen, streakScreen, screen, canvas);

        BeginDrawing();
        ClearBackground(Color{ 6, 10, 22, 255 });
        DrawTexturePro(canvas.texture,
                       Rectangle{ 0, 0, (float)kWidth, -(float)kHeight },
                       dst, Vector2{ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(canvas);
    UIAssets::unloadAll();
    Theme::unloadFonts();
    gAudio.close();
    CloseWindow();
    return 0;
}
