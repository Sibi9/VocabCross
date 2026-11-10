@echo off
cd /d "%~dp0"
if not exist "VocabCross.exe" (
    echo VocabCross.exe is missing. Run build.bat first.
    pause
    exit /b 1
)
start "" "VocabCross.exe"
