#include "raylib.h"
#include "rlgl.h"

#include "audio_manager.hpp"
#include "crossword_puzzle.hpp"
#include "game_data.hpp"
#include "input.hpp"
#include "screen_crossword.hpp"
#include "screen_image_guess.hpp"
#include "screen_maps.hpp"
#include "screen_wheel.hpp"
#include "theme.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

constexpr int kWidth = 540;
constexpr int kHeight = 960;

void DrawFrame(ScreenMaps& map, ScreenCrossword& crossword, ScreenImageGuess& imageGuess,
               ScreenWheel& wheel, int screen, RenderTexture2D canvas) {
    BeginTextureMode(canvas);
    ClearBackground(Color{ 10, 17, 34, 255 });
    switch (screen) {
        case Nav::SCREEN_MAP: map.draw(kWidth, kHeight); break;
        case Nav::SCREEN_CROSSWORD: crossword.draw(kWidth, kHeight); break;
        case Nav::SCREEN_IMAGE_GUESS: imageGuess.draw(kWidth, kHeight); break;
        case Nav::SCREEN_WHEEL: wheel.draw(kWidth, kHeight); break;
        default: break;
    }
    EndTextureMode();
}

void Capture(ScreenMaps& map, ScreenCrossword& crossword, ScreenImageGuess& imageGuess,
             ScreenWheel& wheel, int screen, RenderTexture2D canvas, const std::string& path) {
    for (int i = 0; i < 2; ++i) {
        DrawFrame(map, crossword, imageGuess, wheel, screen, canvas);
    }
    Image img = LoadImageFromTexture(canvas.texture);
    ImageFlipVertical(&img);
    ExportImage(img, path.c_str());
    UnloadImage(img);
    std::cout << "  [CAPTURED] " << path << std::endl;
}

void ResetAllLevels() {
    for (auto& lvl : gGameData.levels) {
        lvl.unlocked = false;
        lvl.stars = 0;
        lvl.score = 0;
    }
    gGameData.total_stars = 0;
}

int LevelOf(const std::string& word) {
    int idx = gGameData.indexOfWord(word);
    if (idx < 0) {
        std::cerr << "Word not found in data: " << word << std::endl;
        return 0;
    }
    return idx;
}

} // namespace

int main(int argc, char* argv[]) {
    bool verifyMode = false;
    bool selfTest = false;
    std::string startWord;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verify-ui") verifyMode = true;
        if (arg == "--self-test") selfTest = true;
        if (arg == "--word" && i + 1 < argc) startWord = argv[i + 1];
    }

    SetTraceLogLevel((verifyMode || selfTest) ? LOG_WARNING : LOG_INFO);
    unsigned int flags = FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE;
    if (verifyMode || selfTest) flags |= FLAG_WINDOW_HIDDEN;
    SetConfigFlags(flags);
    InitWindow(kWidth, kHeight, "VocabCross - Algerian Baccalaureate");
    SetTargetFPS(60);
    // The entire UI is made of 2D triangle fans / stars.  Disable face culling
    // so clockwise and counter-clockwise fans are both filled.
    rlDisableBackfaceCulling();
    ChangeDirectory(GetApplicationDirectory());

    // The game is portrait 9:16.  A 540x960 window does not fit on every
    // monitor (for example a 1600x900 desktop), so fit it to the current
    // monitor while preserving the aspect ratio.  The canvas is letterboxed
    // and Input::UpdateFromWindow() maps clicks back to 540x960 coordinates.
    if (!verifyMode && !selfTest) {
        int mon = GetCurrentMonitor();
        int mw = GetMonitorWidth(mon);
        int mh = GetMonitorHeight(mon);
        int winH = std::min(960, std::max(480, mh - 110));
        int winW = (int)std::round(winH * 540.0f / 960.0f);
        SetWindowSize(winW, winH);
        SetWindowPosition(std::max(0, (mw - winW) / 2), std::max(0, (mh - winH) / 2 - 20));
        SetWindowMinSize(360, 640);
    }

    Theme::loadFonts();
    gAudio.init();
    gAudio.enabled = Theme::soundOn;

    if (!gGameData.loadData("assets/data/bac_vocabulary.json")) {
        std::cerr << "FATAL: vocabulary data could not be loaded." << std::endl;
        CloseWindow();
        return 1;
    }
    if (gGameData.units.size() != 4 || gGameData.levels.size() != 32) {
        std::cerr << "FATAL: expected 4 units / 32 levels, got " << gGameData.units.size()
                  << " / " << gGameData.levels.size() << std::endl;
        return 1;
    }

    ScreenMaps map;
    map.init();
    ScreenCrossword crossword;
    crossword.init();
    ScreenImageGuess imageGuess;
    imageGuess.init();
    ScreenWheel wheel;
    wheel.init();

    RenderTexture2D canvas = LoadRenderTexture(kWidth, kHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    if (selfTest) {
        int failures = 0;
        auto check = [&](bool cond, const std::string& label) {
            std::cout << (cond ? "  [PASS] " : "  [FAIL] ") << label << std::endl;
            if (!cond) ++failures;
        };
        std::cout << ">>> VocabCross self-test" << std::endl;
        check(gGameData.units.size() == 4, "4 curriculum units");
        check(gGameData.levels.size() == 32, "32 vocabulary levels");
        check(Theme::fontCartoon.glyphCount > 90, "Latin font atlas");
        check(Theme::fontArabic.glyphCount > 120, "Arabic font atlas");

        for (int i = 0; i < gGameData.levelCount(); ++i) {
            CrosswordPuzzle p;
            p.build(gGameData.levels[i].data, gGameData.units[gGameData.levels[i].unit_index]);
            bool valid = !p.cells.empty() && p.words.size() >= 2;
            for (const auto& c : p.cells) if (c.words.empty()) valid = false;
            std::cout << "  [PUZZLE] " << gGameData.levels[i].data.word
                      << " -> " << p.words.size() << " words / " << p.cells.size() << " cells"
                      << (valid ? "" : "  <-- CHECK") << std::endl;
            check(valid, "crossword layout: " + gGameData.levels[i].data.word);
        }

        std::string shaped = ArabicShaper::reshape("رشوة / ارتشاء");
        int arabicForms = 0;
        for (uint32_t cp : ArabicShaper::utf8ToCodepoints(shaped)) {
            if (cp >= 0xFE70 && cp <= 0xFEFF) ++arabicForms;
        }
        check(arabicForms >= 5, "Arabic shaping produces presentation forms");
        check(shaped != "رشوة / ارتشاء", "Arabic visual reorder is applied");

        for (int i = 0; i < gGameData.levelCount(); ++i) {
            const auto& lvl = gGameData.levels[i];
            check(!lvl.data.word.empty(), "word present: " + lvl.data.word);
            check(!lvl.data.translation_ar.empty(), "translation present: " + lvl.data.word);
            check(!lvl.data.definition_en.empty(), "definition present: " + lvl.data.word);
            check(!lvl.data.synonym_en.empty(), "synonym present: " + lvl.data.word);
            gGameData.current_level = i;
            if (lvl.data.mode == "crossword") {
                crossword.loadLevel(i);
                check(!crossword.puzzle.words.empty() &&
                      crossword.puzzle.words[0].answer == lvl.data.word,
                      "crossword load: " + lvl.data.word);
                check(crossword.puzzle.words.size() >= 2, "crossword has crossing words: " + lvl.data.word);
                check(crossword.puzzle.cells.size() >= lvl.data.word.size(),
                      "crossword cells: " + lvl.data.word);
                crossword.puzzle.solveAll();
                crossword.complete();
                check(crossword.victoryModal.active, "crossword victory: " + lvl.data.word);
                crossword.victoryModal.hide();
            } else if (lvl.data.mode == "image") {
                imageGuess.loadLevel(i);
                check(imageGuess.targetWord == lvl.data.word, "image load: " + lvl.data.word);
                imageGuess.currentGuess = imageGuess.targetWord;
                imageGuess.complete();
                check(imageGuess.victoryModal.active, "image victory: " + lvl.data.word);
                imageGuess.victoryModal.hide();
            } else {
                wheel.loadLevel(i);
                check(!wheel.puzzle.words.empty() &&
                      wheel.puzzle.words[0].answer == lvl.data.word,
                      "wheel load: " + lvl.data.word);
                check(wheel.puzzle.words.size() >= 2, "wheel has crossing words: " + lvl.data.word);
                check(wheel.puzzle.cells.size() >= lvl.data.word.size(),
                      "wheel cells: " + lvl.data.word);
                wheel.setDemoSelection(lvl.data.word);
                wheel.demoSubmitSelection();
                check(wheel.puzzle.words[0].solved, "wheel word matching: " + lvl.data.word);
                wheel.puzzle.solveAll();
                wheel.complete();
                check(wheel.victoryModal.active, "wheel victory: " + lvl.data.word);
                wheel.victoryModal.hide();
            }
            Theme::particles.clear();
        }
        gGameData.resetProgress();
        std::cout << (failures == 0 ? ">>> SELF-TEST PASS" : ">>> SELF-TEST FAIL") << std::endl;
        UnloadRenderTexture(canvas);
        map.unload();
        crossword.unload();
        imageGuess.unload();
        wheel.unload();
        gAudio.close();
        Theme::unloadFonts();
        CloseWindow();
        return failures == 0 ? 0 : 2;
    }

    if (verifyMode) {
        std::cout << ">>> VocabCross --verify-ui : capturing real engine screenshots" << std::endl;
        std::filesystem::create_directories("verification_screenshots");
        std::filesystem::create_directories("verification_reports");
        Theme::particles.clear();

        // 01 - Ethics island, first node active, rest locked (matches the reference state).
        ResetAllLevels();
        gGameData.levels[0].unlocked = true;
        gGameData.current_level = 0;
        map.unit = 0;
        map.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_MAP, canvas,
                "verification_screenshots/01_map_ethics.png");

        // 02 - Safety island, first node active.
        ResetAllLevels();
        int safetyStart = gGameData.unitStart(1);
        gGameData.levels[safetyStart].unlocked = true;
        gGameData.current_level = safetyStart;
        map.unit = 1;
        map.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_MAP, canvas,
                "verification_screenshots/02_map_safety.png");

        // 03 - Space island with cleared node 01, checked node 02, active node 03.
        ResetAllLevels();
        int spaceStart = gGameData.unitStart(2);
        gGameData.levels[spaceStart].unlocked = true;
        gGameData.levels[spaceStart].stars = 3;
        gGameData.levels[spaceStart + 1].unlocked = true;
        gGameData.levels[spaceStart + 1].stars = 1;
        gGameData.levels[spaceStart + 2].unlocked = true;
        gGameData.current_level = spaceStart + 2;
        map.unit = 2;
        map.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_MAP, canvas,
                "verification_screenshots/03_map_space.png");

        // 04 / 05 - two crossword levels with different words and lengths.
        Theme::particles.clear();
        int bribery = LevelOf("BRIBERY");
        gGameData.current_level = bribery;
        crossword.loadLevel(bribery);
        crossword.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_CROSSWORD, canvas,
                "verification_screenshots/04_crossword_lvl1.png");

        int nepotism = LevelOf("NEPOTISM");
        gGameData.current_level = nepotism;
        crossword.loadLevel(nepotism);
        crossword.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_CROSSWORD, canvas,
                "verification_screenshots/05_crossword_lvl2.png");

        // 06 / 07 - image guess with a different clue image and a different length.
        Theme::particles.clear();
        int eclipse = LevelOf("ECLIPSE");
        gGameData.current_level = eclipse;
        imageGuess.loadLevel(eclipse);
        imageGuess.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_IMAGE_GUESS, canvas,
                "verification_screenshots/06_image_guess_lvl1.png");

        int counterfeit = LevelOf("COUNTERFEIT");
        gGameData.current_level = counterfeit;
        imageGuess.loadLevel(counterfeit);
        imageGuess.time = 0.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_IMAGE_GUESS, canvas,
                "verification_screenshots/07_image_guess_lvl2.png");

        // 08 / 11 - wheel with a live laser path; two different words.
        Theme::particles.clear();
        int galaxy = LevelOf("GALAXY");
        gGameData.current_level = galaxy;
        wheel.loadLevel(galaxy);
        wheel.time = 0.0f;
        wheel.setDemoSelection("GALAXY");
        wheel.demoSubmitSelection();
        wheel.hintToastTimer = 0.0f;
        if (wheel.puzzle.words.size() > 1) {
            std::string comp = wheel.puzzle.words[1].answer;
            wheel.setDemoSelection(comp.substr(0, std::min<size_t>(3, comp.size())));
        }
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_WHEEL, canvas,
                "verification_screenshots/08_wheel_lvl1.png");

        int preservative = LevelOf("PRESERVATIVE");
        gGameData.current_level = preservative;
        wheel.loadLevel(preservative);
        wheel.time = 0.0f;
        wheel.setDemoSelection("PRESERVATIVE");
        wheel.demoSubmitSelection();
        wheel.hintToastTimer = 0.0f;
        if (wheel.puzzle.words.size() > 1) {
            std::string comp = wheel.puzzle.words[1].answer;
            wheel.setDemoSelection(comp.substr(0, std::min<size_t>(3, comp.size())));
        }
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_WHEEL, canvas,
                "verification_screenshots/11_wheel_lvl2.png");

        // 09 - victory modal with the shaped Arabic translation.
        Theme::particles.clear();
        gGameData.current_level = bribery;
        crossword.loadLevel(bribery);
        crossword.puzzle.solveAll();
        crossword.complete();
        crossword.victoryModal.anim = 1.0f;
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_CROSSWORD, canvas,
                "verification_screenshots/09_arabic_victory_modal.png");

        // 12 - dedicated Arabic shaping / diacritics QA card.
        {
            BeginTextureMode(canvas);
            ClearBackground(Color{ 10, 17, 34, 255 });
            DrawRectangleGradientV(0, 0, kWidth, kHeight, Color{ 17, 30, 56, 255 }, Color{ 8, 14, 28, 255 });
            Theme::text3D("ARABIC SHAPING / TASHKEEL", { kWidth * 0.5f, 54.0f }, 24.0f,
                          Theme::goldLight, Color{ 76, 40, 20, 255 }, 1.6f, Theme::WithAlpha(BLACK, 0.5f),
                          true, true);
            const char* words[] = { "GALAXY", "FAKE", "SMUGGLE", "NEPOTISM" };
            for (int i = 0; i < 4; ++i) {
                int idx = gGameData.indexOfWord(words[i]);
                if (idx < 0) continue;
                Rectangle card{ 40.0f, 130.0f + i * 190.0f, 460.0f, 150.0f };
                UI::GlassPanel(card, 18.0f, Color{ 24, 40, 70, 230 }, Theme::WithAlpha(Theme::gold, 0.5f));
                Theme::text3D(Theme::fontCartoon, words[i], { card.x + 18.0f, card.y + 14.0f }, 15.0f,
                              Theme::goldLight, Color{ 76, 40, 20, 255 }, 1.2f, Theme::WithAlpha(BLACK, 0.5f));
                UI::TextArabic(gGameData.levels[idx].data.translation_ar,
                               { card.x + card.width * 0.5f, card.y + 64.0f }, 32.0f,
                               Color{ 255, 251, 235, 255 }, true, true);
                Theme::text(gGameData.levels[idx].data.definition_en,
                            { card.x + card.width * 0.5f, card.y + 122.0f }, 11.0f,
                            Theme::WithAlpha(WHITE, 0.65f), true);
            }
            EndTextureMode();
            Image img = LoadImageFromTexture(canvas.texture);
            ImageFlipVertical(&img);
            ExportImage(img, "verification_screenshots/12_arabic_shaping.png");
            UnloadImage(img);
            std::cout << "  [CAPTURED] verification_screenshots/12_arabic_shaping.png" << std::endl;
        }

        // 10 - the fourth island (Feelings & Humor) is a first-class map too.
        ResetAllLevels();
        int feelingsStart = gGameData.unitStart(3);
        gGameData.levels[feelingsStart].unlocked = true;
        gGameData.levels[feelingsStart + 1].unlocked = true;
        gGameData.levels[feelingsStart + 1].stars = 2;
        gGameData.current_level = feelingsStart;
        map.unit = 3;
        map.time = 0.0f;
        Theme::particles.clear();
        Capture(map, crossword, imageGuess, wheel, Nav::SCREEN_MAP, canvas,
                "verification_screenshots/10_map_feelings.png");

        std::cout << ">>> Captured 12 screenshots from the live engine." << std::endl;
    } else {
        int currentScreen = Nav::SCREEN_MAP;
        if (!startWord.empty()) {
            int idx = gGameData.indexOfWord(startWord);
            if (idx >= 0) {
                gGameData.current_level = idx;
                currentScreen = Nav::stageScreenForLevel(idx);
                if (currentScreen == Nav::SCREEN_CROSSWORD) crossword.loadLevel(idx);
                else if (currentScreen == Nav::SCREEN_IMAGE_GUESS) imageGuess.loadLevel(idx);
                else if (currentScreen == Nav::SCREEN_WHEEL) wheel.loadLevel(idx);
            } else {
                std::cerr << "Unknown --word: " << startWord << std::endl;
            }
        } else {
            map.setUnit(0);
        }
        while (!WindowShouldClose()) {
            float dt = GetFrameTime();
            Input::UpdateFromWindow();
            Theme::timeNow += dt;
            Theme::updateParticles(dt);
            map.update(dt);
            crossword.update(dt);
            imageGuess.update(dt);
            wheel.update(dt);

            int nextScreen = currentScreen;
            switch (currentScreen) {
                case Nav::SCREEN_MAP: map.handleInput(kWidth, kHeight, nextScreen); break;
                case Nav::SCREEN_CROSSWORD: crossword.handleInput(kWidth, kHeight, nextScreen); break;
                case Nav::SCREEN_IMAGE_GUESS: imageGuess.handleInput(kWidth, kHeight, nextScreen); break;
                case Nav::SCREEN_WHEEL: wheel.handleInput(kWidth, kHeight, nextScreen); break;
                default: break;
            }

            if (nextScreen != currentScreen) {
                if (nextScreen == Nav::SCREEN_MAP) {
                    map.setUnit(gGameData.unitOfLevel(gGameData.current_level));
                } else if (nextScreen == Nav::SCREEN_CROSSWORD) {
                    crossword.loadLevel(gGameData.current_level);
                } else if (nextScreen == Nav::SCREEN_IMAGE_GUESS) {
                    imageGuess.loadLevel(gGameData.current_level);
                } else if (nextScreen == Nav::SCREEN_WHEEL) {
                    wheel.loadLevel(gGameData.current_level);
                }
                currentScreen = nextScreen;
            }

            DrawFrame(map, crossword, imageGuess, wheel, currentScreen, canvas);

            BeginDrawing();
            ClearBackground(Color{ 5, 8, 18, 255 });
            Rectangle dst = Input::CanvasDestination(GetScreenWidth(), GetScreenHeight());
            DrawTexturePro(canvas.texture,
                           { 0.0f, 0.0f, (float)kWidth, -(float)kHeight },
                           dst,
                           { 0, 0 }, 0.0f, WHITE);
            EndDrawing();
        }
    }

    UnloadRenderTexture(canvas);
    map.unload();
    crossword.unload();
    imageGuess.unload();
    wheel.unload();
    gAudio.close();
    Theme::unloadFonts();
    CloseWindow();
    return 0;
}
