#!/usr/bin/env python3
"""Rename Rumble* names to Arena*/ToggleTimer* in docs and hbtestd."""
import os
import re

DOCS_DIR = "/home/evan/hamsterball-re/docs"
HBTESTD_FILES = [
    "/home/evan/hamsterball-re/tools/hbtestd/hbtestd/addresses.py",
    "/home/evan/hamsterball-re/tools/hbtestd/hbtestd/server.py",
]

# Replacement rules — applied in order, longest matches first.
# Each rule is a (pattern, replacement) pair using regex.
# The key distinction: when "RumbleBoard" is used as the struct/class name, replace with "ArenaBoard".
# When "RumbleScore" is used, replace with "ArenaScoreParticle".
# When "RumbleObject" is used, replace with "ArenaObject".
# When "RumbleCollision" is used, replace with "ArenaCollisionLevel".
# When "PauseRumbleMenu" is used, replace with "PauseArenaMenu".
# When "RumbleBoard_InitTimer/TickTimer/CleanupTimer" → "ToggleTimer_Init/Tick/Cleanup"
# When "RumbleBoard_WarmUp_Init" → "ArenaLevel_WarmUp_Init"
# etc.

REPLACEMENTS = [
    # --- ToggleTimer functions (check before generic RumbleBoard_ patterns) ---
    (r'\bRumbleBoard_InitTimer\b', 'ToggleTimer_Init'),
    (r'\bRumbleBoard_TickTimerWrapper\b', 'ToggleTimer_TickWrapper'),
    (r'\bRumbleBoard_TickTimer\b', 'ToggleTimer_Tick'),
    (r'\bRumbleBoard_CleanupTimer\b', 'ToggleTimer_Cleanup'),

    # --- RumbleScore → ArenaScoreParticle ---
    # Function names with _ctor, _ListA_Ctor, _ListB_Ctor
    (r'\bRumbleScoreListA_Ctor\b', 'ArenaScoreParticle_ListA_Ctor'),
    (r'\bRumbleScoreListB_Ctor\b', 'ArenaScoreParticle_ListB_Ctor'),
    (r'\bRumbleScore_ctor\b', 'ArenaScoreParticle_ctor'),
    # RumbleScore as a class/type name
    (r'\bRumbleScore\b', 'ArenaScoreParticle'),

    # --- RumbleObject → ArenaObject ---
    (r'\bRumbleObject_Update\b', 'ArenaObject_Update'),
    (r'\bRumbleObject_DeletingDtor\b', 'ArenaObject_DeletingDtor'),
    (r'\bRumbleObject_Level_Dtor\b', 'ArenaObject_Level_Dtor'),
    (r'\bRumbleObject\b', 'ArenaObject'),

    # --- RumbleCollision → ArenaCollisionLevel ---
    (r'\bRumbleCollision_DeletingDtor\b', 'ArenaCollisionLevel_DeletingDtor'),
    (r'\bRumbleCollision_Level_Dtor\b', 'ArenaCollisionLevel_Dtor'),
    (r'\bRumbleCollision\b', 'ArenaCollisionLevel'),

    # --- PauseRumbleMenu → PauseArenaMenu ---
    (r'\bPauseRumbleMenu_ctor\b', 'PauseArenaMenu_ctor'),
    (r'\bPauseRumbleMenu\b', 'PauseArenaMenu'),

    # --- Arena level init functions: RumbleBoard_{Level}_Init → ArenaLevel_{Level}_Init ---
    (r'\bRumbleBoard_WarmUp_Init\b', 'ArenaLevel_WarmUp_Init'),
    (r'\bRumbleBoard_Warmup_Init\b', 'ArenaLevel_Warmup_Init'),
    (r'\bRumbleBoard_Beginner_Init\b', 'ArenaLevel_Beginner_Init'),
    (r'\bRumbleBoard_Intermediate_Init\b', 'ArenaLevel_Intermediate_Init'),
    (r'\bRumbleBoard_Dizzy_Init\b', 'ArenaLevel_Dizzy_Init'),
    (r'\bRumbleBoard_Tower_Init\b', 'ArenaLevel_Tower_Init'),
    (r'\bRumbleBoard_Up_Init\b', 'ArenaLevel_Up_Init'),
    (r'\bRumbleBoard_Neon_Init\b', 'ArenaLevel_Neon_Init'),
    (r'\bRumbleBoard_Expert_Init\b', 'ArenaLevel_Expert_Init'),
    (r'\bRumbleBoard_Odd_Init\b', 'ArenaLevel_Odd_Init'),
    (r'\bRumbleBoard_Toob_Init\b', 'ArenaLevel_Toob_Init'),
    (r'\bRumbleBoard_Wobbly_Init\b', 'ArenaLevel_Wobbly_Init'),
    (r'\bRumbleBoard_Glass_Init\b', 'ArenaLevel_Glass_Init'),
    (r'\bRumbleBoard_Sky_Init\b', 'ArenaLevel_Sky_Init'),
    (r'\bRumbleBoard_Master_Init\b', 'ArenaLevel_Master_Init'),
    (r'\bRumbleBoard_Impossible_Init\b', 'ArenaLevel_Impossible_Init'),
    # Generic catch for any RumbleBoard_{Level}_Init not explicitly listed
    (r'\bRumbleBoard_(\w+)_Init\b', r'ArenaLevel_\1_Init'),

    # --- Arena level constructors: RumbleBoard_{Level}_Ctor → ArenaBoard_{Level}_Ctor ---
    (r'\bRumbleBoard_Warmup_Ctor\b', 'ArenaBoard_Warmup_Ctor'),
    (r'\bRumbleBoard_WarmUp_Ctor\b', 'ArenaBoard_WarmUp_Ctor'),
    (r'\bRumbleBoard_Beginner_Ctor\b', 'ArenaBoard_Beginner_Ctor'),
    (r'\bRumbleBoard_Intermediate_Ctor\b', 'ArenaBoard_Intermediate_Ctor'),
    (r'\bRumbleBoard_Dizzy_Ctor\b', 'ArenaBoard_Dizzy_Ctor'),
    (r'\bRumbleBoard_Tower_Ctor\b', 'ArenaBoard_Tower_Ctor'),
    (r'\bRumbleBoard_Up_Ctor\b', 'ArenaBoard_Up_Ctor'),
    (r'\bRumbleBoard_Neon_Ctor\b', 'ArenaBoard_Neon_Ctor'),
    (r'\bRumbleBoard_Expert_Ctor\b', 'ArenaBoard_Expert_Ctor'),
    (r'\bRumbleBoard_Odd_Ctor\b', 'ArenaBoard_Odd_Ctor'),
    (r'\bRumbleBoard_Toob_Ctor\b', 'ArenaBoard_Toob_Ctor'),
    (r'\bRumbleBoard_Wobbly_Ctor\b', 'ArenaBoard_Wobbly_Ctor'),
    (r'\bRumbleBoard_Glass_Ctor\b', 'ArenaBoard_Glass_Ctor'),
    (r'\bRumbleBoard_Sky_Ctor\b', 'ArenaBoard_Sky_Ctor'),
    (r'\bRumbleBoard_Master_Ctor\b', 'ArenaBoard_Master_Ctor'),
    (r'\bRumbleBoard_Impossible_Ctor\b', 'ArenaBoard_Impossible_Ctor'),
    # Generic catch for any RumbleBoard_{Level}_Ctor not explicitly listed
    (r'\bRumbleBoard_(\w+)_Ctor\b', r'ArenaBoard_\1_Ctor'),

    # --- RumbleBoard_Stands_ctor → ArenaStands_ctor ---
    (r'\bRumbleBoard_Stands_ctor\b', 'ArenaStands_ctor'),

    # --- RumbleBoard_Object_* → ArenaSceneObj_* ---
    (r'\bRumbleBoard_Object_Tick\b', 'ArenaSceneObj_Tick'),
    (r'\bRumbleBoard_Object_dtor\b', 'ArenaSceneObj_dtor'),
    (r'\bRumbleBoard_Object_DeletingDtor\b', 'ArenaSceneObj_DeletingDtor'),

    # --- RumbleBoard_Menu_dtor → ArenaBoard_Menu_dtor ---
    (r'\bRumbleBoard_Menu_dtor\b', 'ArenaBoard_Menu_dtor'),

    # --- CollSlices: RumbleBoard_CollSlices* → ArenaCollSlices* ---
    (r'\bRumbleBoard_CollSlices(\w*)\b', r'ArenaCollSlices\1'),

    # --- Scalar dtors: RumbleBoard_*_scalar_dtor → ArenaBoard_*_ScalarDtor ---
    (r'\bRumbleBoard_(\w+)_scalar_dtor\b', r'ArenaBoard_\1_ScalarDtor'),

    # --- Remaining RumbleBoard_{function} → ArenaBoard_{function} ---
    # (e.g., RumbleBoard_ctor, RumbleBoard_Update, RumbleBoard_Render, RumbleBoard_TickDown,
    #  RumbleBoard_RenderThenFree, RumbleBoard_dtor, RumbleBoard_DeletingDtor)
    (r'\bRumbleBoard_ctor\b', 'ArenaBoard_ctor'),
    (r'\bRumbleBoard_Update\b', 'ArenaBoard_Update'),
    (r'\bRumbleBoard_Render\b', 'ArenaBoard_Render'),
    (r'\bRumbleBoard_TickDown\b', 'ArenaBoard_TickDown'),
    (r'\bRumbleBoard_RenderThenFree\b', 'ArenaBoard_RenderThenFree'),
    (r'\bRumbleBoard_dtor\b', 'ArenaBoard_dtor'),
    (r'\bRumbleBoard_DeletingDtor\b', 'ArenaBoard_DeletingDtor'),

    # --- RUMBLE_BOARD dict in addresses.py → ARENA_BOARD ---
    # This is for the Python variable name
    (r'\bRUMBLE_BOARD\b', 'ARENA_BOARD'),

    # --- RumbleBoard as a struct name/type → ArenaBoard ---
    # Catch any remaining bare RumbleBoard references
    (r'\bRumbleBoard\b', 'ArenaBoard'),

    # --- Ball_DrawRumbleScoreText → Ball_DrawArenaScoreText ---
    (r'\bBall_DrawRumbleScoreText\b', 'Ball_DrawArenaScoreText'),
]

# Words that should NOT be changed (game mode name "Rodent Rumble", asset names)
# We handle this by NOT replacing "Rumble" alone — only the specific compound names above.

def apply_replacements(text: str) -> str:
    """Apply all replacement rules to a text string."""
    for pattern, replacement in REPLACEMENTS:
        text = re.sub(pattern, replacement, text)
    return text


def process_file(filepath: str, dry_run: bool = False) -> tuple[bool, int]:
    """Process a single file. Returns (changed, num_replacements)."""
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        original = f.read()

    modified = apply_replacements(original)

    if modified != original:
        # Count changes
        changes = sum(1 for a, b in zip(original.split('\n'), modified.split('\n')) if a != b)
        if not dry_run:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(modified)
        return True, changes
    return False, 0


def find_md_files(directory: str) -> list[str]:
    """Find all .md files recursively in a directory."""
    result = []
    for root, dirs, files in os.walk(directory):
        for f in files:
            if f.endswith('.md') or f.endswith('.py'):
                result.append(os.path.join(root, f))
    return result


def main():
    import sys
    dry_run = '--dry-run' in sys.argv

    files = find_md_files(DOCS_DIR)
    files.extend(HBTESTD_FILES)

    total_changed = 0
    total_replacements = 0

    for filepath in sorted(files):
        changed, count = process_file(filepath, dry_run=dry_run)
        if changed:
            total_changed += 1
            total_replacements += count
            status = "DRY RUN" if dry_run else "UPDATED"
            print(f"  [{status}] {filepath} ({count} lines changed)")

    print(f"\n{'Dry run: ' if dry_run else ''}Files changed: {total_changed}, Lines modified: {total_replacements}")


if __name__ == '__main__':
    main()
