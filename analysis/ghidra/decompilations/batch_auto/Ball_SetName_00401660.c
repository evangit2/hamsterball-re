/*
 * Function: Ball_SetName
 * Address: 0x00401660
 * Signature: void __thiscall Ball_SetName(void *this, char *param_1)
 *
 * Description:
 * Sets the display name of a ball by making a heap-allocated copy of the input string.
 *
 * Logic:
 *   1. Frees any existing name string at ball+0xC28 (if non-null)
 *   2. Manually strlen's param_1 by iterating until '\0' (Ghidra's decompiler inlines
 *      strlen into a do-while loop)
 *   3. Allocates strlen+1 bytes via operator_new
 *   4. Stores the new string pointer at ball+0xC28
 *   5. Copies the string character-by-character (including null terminator)
 *   6. Sets ball+0x0C to 200 (this is likely a display state or color index —
 *      the value 200 may indicate "name changed" or a default color/score)
 *
 * Struct offsets:
 *   ball+0x0C:  Display state or color index (set to 200)
 *   ball+0xC28: Name string pointer (heap-allocated, freed on re-set or destructor)
 *
 * Cross-references:
 *   - Called from Ball_Update (0x405E00) — likely during name initialization or events
 *   - Called from CheckArenaUnlock (0x40ABA0) — when arena mode unlocks a new ball
 *   - Called from DispatchCollisionEvents (0x40C5D0) — when creating NoDizzy game objects
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_SetName(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  
  if (*(void **)((int)this + 0xc28) != (void *)0x0) {
    _free(*(void **)((int)this + 0xc28));
  }
  pcVar2 = param_1;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  pcVar2 = operator_new((uint)(pcVar2 + (1 - (int)(param_1 + 1))));
  *(char **)((int)this + 0xc28) = pcVar2;
  do {
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    *pcVar2 = cVar1;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  *(undefined4 *)((int)this + 0xc) = 200;
  return;
}
