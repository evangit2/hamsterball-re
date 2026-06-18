@echo off
REM Hamsterball Water Physics Mod uninstaller

if exist "bass_real.dll" (
    echo Restoring original bass.dll ...
    del bass.dll
    ren bass_real.dll bass.dll
    echo Uninstalled.
) else (
    echo bass_real.dll not found. Cannot uninstall automatically.
)
pause
