@echo off
setlocal EnableDelayedExpansion

echo ============================================
echo   Hamsterball FPS Mod Installer (bass.dll)
echo ============================================
echo.

set "GAME_DIR=%~dp0"
set "BASS=%GAME_DIR%bass.dll"
set "BASS_REAL=%GAME_DIR%bass_real.dll"
set "PROXY=%GAME_DIR%hamsterball_fps_mod.dll"
set "INI=%GAME_DIR%hamsterball_fps.ini"

if not exist "%BASS%" (
    echo ERROR: bass.dll not found in %GAME_DIR%
    echo Make sure this batch file is in your Hamsterball game folder.
    pause
    exit /b 1
)

if exist "%BASS_REAL%" (
    echo bass_real.dll already exists - mod may already be installed.
    choice /C YN /M "Reinstall anyway"
    if errorlevel 2 exit /b 0
)

if exist "%PROXY%" (
    echo Installing proxy DLL...
) else (
    echo ERROR: hamsterball_fps_mod.dll not found.
    echo Please copy the mod files into this folder first.
    pause
    exit /b 1
)

:: Rename original bass.dll -> bass_real.dll
echo Backing up original bass.dll to bass_real.dll...
ren "%BASS%" "bass_real.dll" >nul 2>&1
if exist "%BASS%" (
    echo FAILED: could not rename bass.dll. Run as administrator?
    pause
    exit /b 1
)

:: Copy proxy as bass.dll
echo Installing proxy bass.dll...
copy /Y "%PROXY%" "%BASS%" >nul
if not exist "%BASS%" (
    echo FAILED: could not copy proxy bass.dll
    pause
    exit /b 1
)

:: Create default INI if missing
if not exist "%INI%" (
    echo Creating default hamsterball_fps.ini...
    (
        echo [FPS]
        echo TargetFPS=144
        echo RenderFPS=144
        echo.
        echo [Uncap]
        echo Uncap=0
    ) > "%INI%"
)

echo.
echo Installation complete!
echo.
echo Edit %INI% to change your FPS cap.
echo.
pause
