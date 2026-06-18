@echo off
REM Hamsterball Water Physics Mod installer
REM Run this from your Hamsterball game folder.

if not exist "Hamsterball.exe" (
    echo Error: Hamsterball.exe not found in this folder.
    echo Please run install_water_mod.bat from your Hamsterball game directory.
    pause
    exit /b 1
)

if not exist "bass_real.dll" (
    if exist "bass.dll" (
        echo Backing up original bass.dll to bass_real.dll ...
        ren bass.dll bass_real.dll
    ) else (
        echo Error: original bass.dll not found. The mod requires the real BASS library.
        pause
        exit /b 1
    )
)

echo Installing water physics mod ...
copy /Y bass.dll.bak 2>nul
copy /Y bass.dll bass.dll.bak 2>nul
copy /Y "%~dp0bass.dll" .
copy /Y "%~dp0hamsterball_water.ini" .

echo Done. Run Hamsterball.exe normally.
pause
