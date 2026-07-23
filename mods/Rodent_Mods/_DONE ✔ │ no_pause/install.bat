@echo off
title No-Pause Mod Installer
echo ============================================
echo   Hamsterball No-Pause Mod Installer
echo ============================================
echo.
echo This mod prevents the pause menu from appearing when you press ESC.
echo The game will continue running normally instead of pausing.
echo.

cd /d "%~dp0"

if not exist "bass.dll" (
    echo ERROR: bass.dll not found in mod folder.
    echo Make sure you extracted the zip correctly.
    pause
    exit /b 1
)

if not exist "..\Hamsterball.exe" (
    echo ERROR: Hamsterball.exe not found in parent directory.
    echo This installer must be run from inside the Hamsterball game folder.
    pause
    exit /b 1
)

cd ..

if exist "bass_real.dll" (
    echo Another bass.dll proxy mod appears to be installed.
    echo Removing old proxy bass.dll...
    del bass.dll
)

echo Backing up original bass.dll to bass_real.dll...
ren bass.dll bass_real.dll

echo Installing No-Pause proxy bass.dll...
copy "%~dp0bass.dll" bass.dll

echo.
echo ============================================
echo   Installation complete!
echo   Press ESC during a race - nothing happens!
echo ============================================
echo.
echo To uninstall: delete bass.dll and rename bass_real.dll back to bass.dll
pause
