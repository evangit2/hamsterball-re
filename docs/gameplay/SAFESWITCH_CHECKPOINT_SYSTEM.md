# SAFESWITCH Checkpoint System

The checkpoint/respawn-point selection system in Hamsterball. Allows level designers
to place named respawn markers along the track so the ball respawns at the last
checkpoint it passed, instead of going all the way back to START.

## Overview

The system has two components placed in the level's MESHWORLD file:

1. **SAFESPOT objects** — invisible reference points placed at specific track positions.
   They can carry a letter tag in parentheses (e.g. `SAFESPOT(B)`) or square brackets
   (e.g. `SAFESPOT[X]`) to enable filtered respawn selection.

2. **E:SAFESWITCH(X) events** — invisible collision trigger volumes placed along the
   track. When the ball touches one, the letter parameter is copied into a filter
   field on the ball, changing which SAFESPOTs are valid respawn candidates.

When the ball falls off the track and hits an `E:LIMIT` boundary, the respawn system
uses the current filter to find the nearest matching SAFESPOT — creating a
"last checkpoint" progression system.

## Binary Implementation

### E:SAFESWITCH Handler (DispatchCollisionEvents @ 0x40C5D0)

**Address:** 0x40C6FC in `DispatchCollisionEvents`

The handler performs a `stricmp` against `"E:SAFESWITCH"` (string at 0x4CF8A8).
If the event string matches, it executes the following logic:

```c
// Pseudocode of the SAFESWITCH handler at 0x40C6FC

if (stricmp(event_string, "E:SAFESWITCH") == 0) {
    char* paren = strchr(event_string, '(');   // 0x4BACC0 = strchr, 0x28 = '('
    
    if (paren == NULL) {
        // No letter parameter — clear the filter
        ball->c2c_filter[0] = '\0';              // ball+0xC2C = empty string
    } else {
        // Copy "(X)" from event string into ball+0xC2C
        // (byte-by-byte copy until null terminator)
        char* dst = ball + 0xC2C;
        char* src = paren;
        do {
            *dst = *src;
            dst++; src++;
        } while (*src != '\0');
    }
    
    // FALL THROUGH to E:LIMIT handler (no return/jump past it)
    // This means SAFESWITCH ALSO deactivates the ball:
    ball->is_active = 0;         // +0x768 = 0
    ball->death_pending = 1;        // +0x2E9 = 1
    // ... arena scoring logic if applicable ...
}
```

**Key behavior:** E:SAFESWITCH always falls through to the E:LIMIT handler. This means
touching a SAFESWITCH volume immediately deactivates the ball AND sets the checkpoint
filter — the ball doesn't just pass through and continue racing. The deactivation
triggers the respawn cycle, which uses the newly-set filter to select the respawn point.

### Ball Filter Field

| Offset | Type | Size | Description |
|--------|------|------|-------------|
| `ball+0xC2C` | char[] | ~8 bytes | SAFESWITCH filter string (e.g. `"(B)"`, `"(C)"`, or empty `""`) |

- **Empty string** — no SAFESWITCH has been hit yet (or a plain `E:SAFESWITCH` with no
  letter was hit). The respawn search accepts any plain SAFESPOT (rejects `[X]`/`[Z]` tagged ones).
- **"(B)"** — the ball hit an `E:SAFESWITCH(B)` volume. The respawn search only accepts
  SAFESPOTs whose name contains `(B)`.
- Set at construction to empty. Persists across respawns until changed by another SAFESWITCH.

### Respawn Point Filter Logic (Ball_FindClosestRespawnPoint @ 0x405190)

The filter logic is in the respawn-point search loop (decompiled lines 121-144):

```c
// For each SAFESPOT candidate in the board's respawn list:

// 1. Calculate filter string length
int filter_len = strlen(ball->c2c_filter);   // ball+0xC2C

if (filter_len == 0) {
    // NO FILTER — accept this SAFESPOT unless it has [Z] or [X] tags
    bool accept = true;
    if (strstr(safespot_name, "[Z]") != NULL) accept = false;
    if (strstr(safespot_name, "[X]") != NULL) accept = false;
    
} else {
    // FILTER ACTIVE — check if SAFESPOT name contains '('
    char* paren = strchr(safespot_name, '(');
    if (paren != NULL) {
        // Compare first 2 chars: "(B" from SAFESPOT name vs "(B" from filter
        if (strnicmp(paren, ball->c2c_filter, 2) == 0) {
            // Match! Accept this SAFESPOT (subject to [Z]/[X] check below)
            goto accept_safespot;
        }
        // No match — skip this SAFESPOT
    }
    // If no '(' in SAFESPOT name, skip it
}

// At accept_safespot label:
// Check [Z]/[X] tags (same as no-filter path)
// Then compute 3D distance from ball to SAFESPOT
// Track the closest matching SAFESPOT
```

**The 2-character `strnicmp` comparison** compares the first two characters after `(`:
- `"(B"` from `SAFESPOT(B)` vs `"(B"` from `ball+0xC2C` → match (case-insensitive)
- `"(B"` from `SAFESPOT(B)` vs `"(C"` from `ball+0xC2C` → no match

This means the filter only checks the letter, not the closing paren. `"(B)"` in the
filter matches `"(B)"` in the SAFESPOT name, but the comparison is only on `"(B"`.

### Gravity-Mode Interaction (ball+0x748)

The respawn search has three modes based on `ball+0x748` (gravity plane index):

| Value | Gravity | Search Behavior | Levels |
|-------|---------|----------------|--------|
| 0 | Y-up (standard) | Iterates board's SAFESPOT list normally | Most races |
| 1 | X-axis (tilted) | Uses random selection from SAFESPOT list | Up Race (LevelUp) |
| 2 | Z-axis (flat) | Uses random selection from SAFESPOT list | Odd Race (Level6) |

In mode 0, the search iterates ALL SAFESPOTs and finds the closest by 3D distance.
In modes 1 and 2, the search picks a random SAFESPOT from the list (using `RNG_Rand`).

**Multiplayer proximity check:** In 2-player mode (`App+0x234` flag), the search also
checks if the other player's ball is within `ball+0x284` (ball radius) of the SAFESPOT.
If so, that SAFESPOT is rejected to prevent respawning on top of the other player.

## Per-Level SAFESPOT/SAFESWITCH Inventory

Verified from binary strings extraction of each level's `.MESHWORLD` file:

| Race | Level File | SAFESPOTs | E:SAFESWITCH Triggers |
|------|-----------|-----------|----------------------|
| 1 Warm-Up | Level1 | SAFESPOT (plain) | — |
| 2 Beginner | LevelCascade | SAFESPOT (plain) | — |
| 3 Intermediate | Level2 | SAFESPOT (plain) | — |
| 4 Dizzy | Level3 | SAFESPOT (plain) | — |
| 5 Tower | Level4 | SAFESPOT, (A), (B) | E:SAFESWITCH, (A), (B) |
| 6 Up | LevelUp | SAFESPOT (plain) | — |
| 7 Neon | LevelDark | SAFESPOT, (A) | E:SAFESWITCH, (A) |
| 8 Expert | Level5 | SAFESPOT, (A), (B), (C), (D), (E) | E:SAFESWITCH, (A), (B), (C), (D), (E) |
| 9 Odd | Level6 | SAFESPOT, (A), (B), (C), [X], [Z] | E:SAFESWITCH, (A), (B), (C) |
| 10 Toob | Level8 | SAFESPOT (plain) | — |
| 11 Wobbly | Level7 | SAFESPOT, (B), (C) | E:SAFESWITCH(B), E:SAFESWITCH(C) |
| 12 Glass | LevelGlass | SAFESPOT (plain) | — |
| 13 Sky | Level9 | SAFESPOT, (B), (C), (D), (E), (F), (G), (H) | E:SAFESWITCH(B) through (H) |
| 14 Master | Level10 | SAFESPOT, (A), (C), (D), (Q), (X), (Z) | E:SAFESWITCH, (A), (C), (D), (Q), (X), (Z) |
| 15 Impossible | LevelImpossible | SAFESPOT, (A), (B) | E:SAFESWITCH, (A), (B) |

### Observations

- **Early races (1-4, 6, 10, 12)** have no checkpoint system — only plain `SAFESPOT`.
  The ball respawns at the nearest one regardless of progress.
- **Later races (5, 7-9, 11, 13-15)** use lettered checkpoints to create progression.
  The ball respawns at the last SAFESPOT whose letter matches the current filter.
- **Sky Race (Level9)** has the most checkpoints: 7 lettered SAFESPOTs (B through H).
- **Master Race (Level10)** uses unusual letters: Q, X, Z — alongside A, C, D.
- **Odd Race (Level6)** is unique in having both `()` and `[]` tagged SAFESPOTs:
  - `(A)`, `(B)`, `(C)` — filtered by the SAFESWITCH letter system
  - `[X]`, `[Z]` — filtered by the gravity-mode system (ball+0x748), NOT by SAFESWITCH

## Square Bracket Tags ([X], [Z])

Some SAFESPOTs use square brackets instead of parentheses. These are **not** related
to the SAFESWITCH system — they are filtered by the gravity-mode system:

- `SAFESPOT[X]` — only used when `ball+0x748 == 1` (tilted X-axis gravity, Up Race)
- `SAFESPOT[Z]` — only used when `ball+0x748 == 2` (flat Z-axis gravity, Odd Race)

When `ball+0x748 == 0` (standard Y-up gravity), SAFESPOTs with `[X]` or `[Z]` tags
are **rejected** during the search. This prevents a Y-up race from using a tilted-gravity
respawn point that might be positioned in an unexpected location.

Currently, only **Odd Race (Level6)** has both `()` and `[]` tagged SAFESPOTs in the
same level file, because it uses gravity mode 2 (Z-axis) which has a different search
behavior than the standard mode 0.

## Complete Respawn Flow

When a ball falls off the track:

1. **E:LIMIT or E:SAFESWITCH collision** → `ball+0x768 = 0` (deactivate), `ball+0x2E9 = 1` (death_pending)
2. **Ball_FallUpdate (0x408830)** countdown: `ball+0xC60 -= 0.02` per frame
3. **Timer expires** (`ball+0xC60 < 0.0`) → `ball+0x2E8 = 1` (needs_respawn)
4. **Ball_FindClosestRespawnPoint (0x405190)** runs:
   - Resets ball state (alpha=0, is_stunned=1, invisible)
   - Reads `ball+0x748` to determine search mode (0=nearest, 1/2=random)
   - Reads `ball+0xC2C` to determine filter string
   - Iterates SAFESPOT list on `board+0x1518` (AthenaList)
   - For each SAFESPOT: check filter match, check [X]/[Z] tags, compute distance
   - Selects closest matching SAFESPOT (or random in modes 1/2)
   - Teleports ball to selected SAFESPOT position
5. **Ball_Update (0x405E00)** recovery: `alpha` grows from 0.0 back to 1.0
6. When `alpha >= 1.0`: `is_stunned = 0`, ball is interactive again

## Modding Implications

- **To add checkpoints to a custom level:** Place `SAFESPOT(X)` reference points
  and `E:SAFESWITCH(X)` collision volumes in the MESHWORLD file. The engine handles
  the rest automatically.
- **To change which SAFESPOT a ball respawns at:** Write a new filter string to
  `ball+0xC2C` (e.g. `"(C)"`). The next respawn will search for `SAFESPOT(C)`.
- **To force respawn at START:** Clear `ball+0xC2C` to empty string. The search
  will find the nearest plain SAFESPOT or START point.
- **To disable checkpoint progression:** Hook the SAFESWITCH handler at 0x40C6FC
  and NOP the filter-copy loop, or always clear `ball+0xC2C` after each respawn.

## See Also

- [Ball Respawn State Machine](../../analysis/ghidra/decompilations/ball/decomp_0x408830_Ball_FallUpdate.c)
  — Fall timer and respawn trigger logic
- [Ball_FindClosestRespawnPoint decompilation](../../analysis/ghidra/decompilations/batch_auto/Ball_FindClosestRespawnPoint_0x00405190.c)
  — Full search algorithm with filter logic
- [Collision Event Dispatch](COLLISION_EVENT_DISPATCH.md)
  — Where E:SAFESWITCH and E:LIMIT handlers live in DispatchCollisionEvents
- [Level Reference](LEVEL_REFERENCE.md)
  — Per-level file and race mapping
