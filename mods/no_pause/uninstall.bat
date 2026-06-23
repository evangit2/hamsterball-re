@echo off
title No-Pause Mod Uninstaller
echo ============================================
echo   Hamsterball No-Pause Mod Uninstaller
echo ============================================
echo.

cd /d "%~dp0\.."

if not exist "bass_real.dll" (
    echo ERROR: bass_real.dll not found. Original backup is missing.
    echo You may need to reinstall Hamsterball to restore the original bass.dll.
    pause
    exit /b 1
)

echo Removing No-Pause proxy bass.dll...
del bass.dll

echo Restoring original bass.dll...
ren bass_real.dll bass.dll

echo.
echo ============================================
echo   Uninstall complete! Pause menu restored.
echo ============================================
pause
