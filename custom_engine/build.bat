@echo off
setlocal
cd /d "%~dp0"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSPATH="
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
)
if not defined VSPATH (
    if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
        set "VSPATH=C:\Program Files\Microsoft Visual Studio\18\Community"
    )
)
if not defined VSPATH (
    echo [ERROR] Visual Studio C++ build tools were not found.
    pause
    exit /b 1
)

call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
    echo [ERROR] vcvars64.bat failed.
    pause
    exit /b 1
)

echo Building VocabCross.exe ...
cl /nologo /std:c++17 /O2 /EHsc /MD /utf-8 /Iinclude src\main.cpp /Fe:VocabCross.exe ^
   /link /LIBPATH:lib raylib.lib winmm.lib gdi32.lib user32.lib shell32.lib opengl32.lib ole32.lib
if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)
echo [OK] VocabCross.exe built.
