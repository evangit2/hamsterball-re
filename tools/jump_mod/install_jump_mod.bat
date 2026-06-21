@echo off
setlocal EnableDelayedExpansion

echo ============================================
echo   Hamsterball Jump Mod Installer v7
echo ============================================
echo.

set "GAME_DIR=%~dp0"
set "BASS=%GAME_DIR%bass.dll"
set "BASS_REAL=%GAME_DIR%bass_real.dll"
set "PROXY=%GAME_DIR%jump_mod_bass.dll"

:: Check we're in the right directory
if not exist "%GAME_DIR%Hamsterball.exe" (
    echo ERROR: Hamsterball.exe not found in %GAME_DIR%
    echo Please copy this batch file into your Hamsterball game folder.
    pause
    exit /b 1
)

:: Check the proxy DLL exists
if not exist "%PROXY%" (
    echo ERROR: jump_mod_bass.dll not found.
    echo Please copy jump_mod_bass.dll into your Hamsterball game folder.
    pause
    exit /b 1
)

:: If bass_real.dll already exists, another mod is installed
if exist "%BASS_REAL%" (
    echo.
    echo WARNING: bass_real.dll already exists.
    echo This means another bass.dll proxy mod is already installed.
    echo Installing the jump mod will REPLACE the other mod.
    echo.
    choice /C YN /M "Replace existing mod with jump mod"
    if errorlevel 2 exit /b 0
    echo Removing old proxy bass.dll...
    del "%BASS%" >nul 2>&1
) else (
    :: First time: rename original bass.dll to bass_real.dll
    if exist "%BASS%" (
        echo Backing up original bass.dll to bass_real.dll...
        ren "%BASS%" "bass_real.dll" >nul 2>&1
        if exist "%BASS%" (
            echo FAILED: could not rename bass.dll. Try running as administrator.
            pause
            exit /b 1
        )
    ) else (
        echo ERROR: bass.dll not found. Cannot install proxy.
        pause
        exit /b 1
    )
)

:: Copy proxy as bass.dll
echo Installing jump mod bass.dll...
copy /Y "%PROXY%" "%BASS%" >nul
if not exist "%BASS%" (
    echo FAILED: could not copy proxy bass.dll
    pause
    exit /b 1
)

echo.
echo ============================================
echo   Installation complete!
echo.
echo   The jump mod will show a MessageBox when
echo   the game starts, confirming the DLL loaded.
echo.
echo   To uninstall: run uninstall_jump_mod.bat
echo ============================================
echo.
pause
