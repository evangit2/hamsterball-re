# mkn_plus_bumper_quantity

**Author:** MAKYUNI 🦇  
**Contributors:** Hamsterbot  
**HB+ Version:** v2.1 (API v3)  
**Type:** HB+ Plus Mod

## What it does

Increases the hardcoded bumper count limit for all levels that have bumpers.

The game hardcodes how many `N:BUMPER%d` meshbuffers each level's init function scans for. This mod patches the `CMP EAX, <count>` instruction in each level init to allow more bumpers.

## Patched levels

| Level | Address | Original Limit |
|-------|---------|----------------|
| Beginner Race | 0x004111C5 | 8 |
| Toob Race | 0x0040FB06 | 8 |
| Master Race | 0x00412047 | 4 |
| Arena Beginner | 0x00413DC6 | 4 |
| Arena Toob | 0x00414FE6 | 5 |

## Config

Edit `mkn_plus_bumper_quantity.txt` (next to the DLL in `Mods\`):

```
# Set the number of bumpers for all levels
# Range: 1-127
# Default: 16
16
```

Changes are applied live — edit the file while the game is running and the mod will re-read it every frame.

## How to use

1. Place `mkn_plus_bumper_quantity.dll` and `mkn_plus_bumper_quantity.txt` in your `Mods\` folder
2. Enable the mod in HB+ options
3. Edit the `.txt` file to set your desired bumper count

## Technical notes

- Only patches a single byte (the immediate in `CMP EAX, imm8`) at each of the 5 level init functions
- The collision handler has no bounds check — it trusts the level init to have found all meshbuffers, so extra bumpers work automatically
- Max value: 127 (signed byte range for x86 `CMP EAX, imm8`)
- You must have `N:BUMPER%d` meshbuffers in your level file for the extra bumpers to actually appear
