---
name: hamsterball-re-progress
description: Track hamsterball RE project state
version: 2026-04-13
---

# Hamsterball RE Status

Progress: 77.4% (3061/3958 functions documented)

## Session 21 renames
Scene_Render3DObjects, Mesh_Dtor/FindElement/AddElement/ConnectElements, App_GetProductVersion, BitStream_ReadValue, FileHandle_Dtor, Math_AlignUp, Array_CopyElements, DSound_SetVolume, CRT_FreeIfNotNull, Huffman_BuildTable, BitStream_FlushAndReset, Sound_DecodeFrame, D3DTexture_Init/InitLocked/NullDtor, CRT_FreeParam2, Font_DecodeGlyphBits, Path_ComputeSegmentLengths, RaceResults_ctor/dtor/Render/Update/Reset, RaceResultsMenu_ctor/scalar_dtor, StdString_Insert/InsertCStr/TruncateToWidth, App_CreateConfirmMenu, App_CreateHighScoreEntry, Level ctors/dtors (Glass, Impossible, Cascade, WarmUp, Tower, Spinner, Intermediate, Sawblade, Gear), TourneyRaceEntry_Dtor

## Git Push Fix
Local commits won't push via SSH (key not in agent) or via HTTPS (gh credential helper hangs).
Fix: use HTTPS with store credential helper:
```
cd ~/hamsterball-re
git remote set-url origin https://github.com/evangit2/hamsterball-re.git
git config credential.helper 'store --file=/tmp/git-creds-store'
git push
```