/*
 * Function: CreateSecretObjects
 * Address: 0x0040baa0
 *
 * Description:
 *
Factory function that creates SECRET and SECRETUNLOCK game objects from
MeshWorld data. Iterates the MW object list, matching "SECRETUNLOCK" and
"SECRET" name prefixes. For each match:
  - Allocates 0x10EC bytes for a Secret object
  - Calls Secret_ctor with position data and the App's mesh pointer
  - Appends to Scene+0xCD4 (object list) and collision lists
  - For "SECRET": calls Rotator_MarkTriggered
  - For "SECRETUNLOCK": calls CheckArenaUnlock

Only executes if: difficulty != 0, not in practice mode, and not in demo mode.

Cross-refs: 1 call — Scene_SpawnBallsAndObjects.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
