@echo off
title VocabCross - Verification, Build and Launch Console
color 0A
cd /d "%~dp0"

echo ============================================================================
echo   VOCABCROSS ENGINE - AUTOMATED VERIFICATION, BUILD AND LAUNCH
echo   Target Directory: %CD%
echo ============================================================================
echo.

echo [STEP 1/4] Verifying Patched C++ Headers and Clean UI Assets...
for %%H in (
    "include\ui_asset_pack.hpp"
    "include\audio_manager.hpp"
    "include\screen_home.hpp"
    "include\screen_maps.hpp"
    "include\screen_crossword.hpp"
    "include\screen_image_guess.hpp"
    "include\screen_wheel.hpp"
    "include\screen_shop.hpp"
    "include\screen_streak.hpp"
    "include\victory_modal.hpp"
    "include\word_card_modal.hpp"
    "src\main.cpp"
) do (
    if exist %%H (
        echo   [OK] Verified source file: %%~H
    ) else (
        echo   [MISSING] Source file not found: %%~H
    )
)

echo.
echo [STEP 2/4] Building VocabCross.exe in %CD% ...
if exist "VocabCross.exe" (
    taskkill /F /IM VocabCross.exe >nul 2>&1
    del /F /Q "VocabCross.exe" >nul 2>&1
)

if exist "build.bat" (
    echo   [BUILD] Running build.bat ...
    echo. | call build.bat
) else (
    echo   [BUILD] Compiling directly with g++ ...
    g++ -O2 -std=c++17 src/main.cpp -o VocabCross.exe -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
)

echo.
echo [STEP 3/4] Verifying Built Executable Output...
if exist "%CD%\VocabCross.exe" (
    for %%F in ("%CD%\VocabCross.exe") do (
        echo   [SUCCESS] Built executable is ready!
        echo   [OUTPUT PATH] %%~fF
        echo   [FILE SIZE]   %%~zF bytes
        echo   [BUILD TIME]  %%~tF
    )
) else (
    color 0C
    echo   [ERROR] Build failed! VocabCross.exe was not generated in %CD%.
    echo   Please inspect the compiler errors above.
    pause
    exit /b 1
)

echo.
echo [STEP 4/4] Launching VocabCross.exe ...
echo   Opening game window from: %CD%\VocabCross.exe
start "" "%CD%\VocabCross.exe"
echo.
echo ============================================================================
echo   ALL STEPS COMPLETED! VOCABCROSS IS NOW RUNNING.
echo ============================================================================
