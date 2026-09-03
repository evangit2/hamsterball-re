# WobblyBridgeFix — HB+ version

HB+ port of the Wobbly wavy bridge fix (bass.dll → HB+).

*Fixes the same bug:* `Wavy_Update 0x440390` `PUSH 0x2000` at `0x440752` → `PUSH 0` (DISCARD→plain lock). Same 342-vert bridge, same bounds, same patch.

## Install (HB+)

Drop `WobblyBridgeFix.dll` into Hamsterball `Mods/` folder (Hamsterball Plus). Enable **Wobbly Bridge Fix** in Options → Mods (default ON). No `bass_real.dll` dance needed.

## Toggle

`WOBBLY_FIX` button → `PATCH 0x440752` `68 00 20 00 00` ↔ `68 00 00 00 00`. Patched at `Initialize` even if toggle state is saved (HB+ `onButtonToggle` only fires on click, so we patch eagerly and keep `patched` in sync).

## Build

* MSVC (Visual Studio): open `WobblyBridgeFix.cpp` + `HamsterballAPI.h` → DLL
* MinGW (Linux): `./build.sh` → `WobblyBridgeFix.dll`

Source: `WobblyBridgeFix.cpp` (MSVC) + `WobblyBridgeFix_MinGW.cpp` (MinGW, static_assert-patched header).

## Addresses

`Wavy_ctor 0x43AD40` `Wavy_Configure 0x435440` `Wavy_Update 0x440390` `Lock 0x440752` `0x68 00 20 00 00 → 68 00 00 00 00`
