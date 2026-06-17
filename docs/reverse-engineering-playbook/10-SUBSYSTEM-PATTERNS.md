# 10 - Subsystem Patterns

## Graphics

Look for `Direct3DCreate8/9`, `CreateWindowEx`, `RegisterClass`, `wglCreateContext`, `SDL_CreateWindow`.

## Input

Look for `DirectInput8Create`, `GetAsyncKeyState`, `RegisterRawInputDevices`, `XInput`.

## Audio

Look for `DirectSoundCreate`, `BASS_Init`, `waveOutOpen`, OpenAL.

## File Formats

Binary formats often follow:
- Magic / version
- Count fields
- Fixed-size records or length-prefixed strings
- Float arrays (3 or 16 floats for 3D transforms)

## Event Systems

Many older games use flat string comparison chains:
```cpp
if (__stricmp(name, "E:JUMP") == 0) { ... }
if (__stricmp(name, "E:ACTION") == 0) { ... }
```
