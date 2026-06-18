@echo off
setlocal EnableDelayedExpansion

echo ============================================
echo   Hamsterball FPS Mod Uninstaller
echo ============================================
echo.

set "GAME_DIR=%~dp0"
set "BASS=%GAME_DIR%bass.dll"
set "BASS_REAL=%GAME_DIR%bass_real.dll"

if not exist "%BASS_REAL%" (
    echo ERROR: bass_real.dll not found. Nothing to uninstall.
    pause
    exit /b 1
)

:: Remove proxy
del "%BASS%" >nul 2>&1
if exist "%BASS%" (
    echo FAILED: could not remove proxy bass.dll. Close Hamsterball first.
    pause
    exit /b 1
)

:: Restore original
ren "%BASS_REAL%" "bass.dll" >nul 2>&1
if not exist "%BASS%" (
    echo FAILED: could not restore original bass.dll
    pause
    exit /b 1
)

echo Mod uninstalled. Original bass.dll restored.
pause
