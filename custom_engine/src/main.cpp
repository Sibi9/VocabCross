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

} // namespace

int main(int argc, char* argv[]) {
    bool verifyMode = false;
    bool selfTest = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verify-ui" || arg == "--verify") verifyMode = true;
        if (arg == "--self-test") selfTest = true;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(kWidth, kHeight, "BAC Vocab Quest - VocabCross Engine");
    SetTargetFPS(60);

    gAudio.init();
    Theme::initFonts();
    UIAssets::init();

    if (!gGameData.loadFromFile("assets/data/bac_vocabulary.json")) {
        gGameData.loadFromFile("../data/bac_vocabulary.json");
    }

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

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    if (verifyMode) {
        std::cout << "[VERIFY-UI] Capturing all 14 UI screens and modals..." << std::endl;
        std::filesystem::create_directories("verification_screenshots");

        // 0. Home Screen (BAC VOCAB QUEST + Animated Trio)
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_HOME, "verification_screenshots/00_home_bac_vocab_quest.png", canvas);

        // 1..3. Original 3 Unit Maps
        mapScreen.setUnit(0);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/01_map_ethics.png", canvas);

        mapScreen.setUnit(1);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/02_map_safety.png", canvas);

        mapScreen.setUnit(2);
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/03_map_space.png", canvas);

        // 4. Graduation Academy Map (Scrolls 1-14 -> Grand Blue Castle)
        mapScreen.setUnit(3);
        mapScreen.boyProgress = 5.0f;
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_MAP, "verification_screenshots/04_map_graduation_academy.png", canvas);

        // 5..6. Crossword (Level 1 & Level 2)
        crosswordScreen.setLevel(LevelOf("BRIBERY"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/04_crossword_lvl1.png", canvas);

        crosswordScreen.setLevel(LevelOf("COUNTERFEIT"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/05_crossword_lvl2.png", canvas);

        // 7..8. Image Guess (Level 1 & Level 2)
        imageGuessScreen.setLevel(LevelOf("ECLIPSE"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_IMAGE_GUESS, "verification_screenshots/06_image_guess_lvl1.png", canvas);

        imageGuessScreen.setLevel(LevelOf("ORGANIC"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_IMAGE_GUESS, "verification_screenshots/07_image_guess_lvl2.png", canvas);

        // 9. Wheel Connect
        wheelScreen.setLevel(LevelOf("GALAXY"));
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_WHEEL, "verification_screenshots/08_wheel_lvl1.png", canvas);

        // 10. Victory Modal (word_graduate_victory_ui_kit)
        if (!gGameData.levels.empty()) {
            crosswordScreen.victoryModal.show(gGameData.levels[LevelOf("BRIBERY")].word);
            crosswordScreen.victoryModal.anim = 1.0f;
        }
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_CROSSWORD, "verification_screenshots/09_arabic_victory_modal.png", canvas);

        // 11. Word Card Modal ("INTEGRITY" - image1_integrity_word_card_ui_kit)
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

        // 12. Booster Shop (magical_alchemist_booster_shop_ui_kit)
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_SHOP, "verification_screenshots/11_booster_shop.png", canvas);

        // 13. Daily Streak 1-7 (daily_streak_roadmap_ui_kit)
        Capture(homeScreen, mapScreen, crosswordScreen, imageGuessScreen, wheelScreen, shopScreen, streakScreen,
                Nav::SCREEN_STREAK, "verification_screenshots/12_daily_streak_1_to_7.png", canvas);

        UnloadRenderTexture(canvas);
        UIAssets::unloadAll();
        Theme::unloadFonts();
        gAudio.close();
        CloseWindow();
        return 0;
    }

    int screen = Nav::SCREEN_HOME;
    if (selfTest) screen = Nav::SCREEN_MAP;

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
                std::string m = gGameData.levels[lvl].mode;
                if (m == "image_guess") {
                    imageGuessScreen.setLevel(lvl);
                    screen = Nav::SCREEN_IMAGE_GUESS;
                } else if (m == "wheel") {
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
