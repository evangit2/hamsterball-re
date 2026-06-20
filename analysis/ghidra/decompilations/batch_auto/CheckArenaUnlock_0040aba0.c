/*
 * Function: CheckArenaUnlock
 * Address: 0x0040aba0
 * Signature: void __fastcall CheckArenaUnlock(int param_1)
 *
 * Patterns: audio, ball. Calls: CheckArenaUnlock, Sound_PlayChannel, Ball_SetName. Offsets: 18, Lines: 56
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall CheckArenaUnlock(int param_1)

{
  int iVar1;
  char *pcVar2;
  
  iVar1 = *(int *)(param_1 + 0x878);
  switch(*(undefined4 *)(*(int *)(iVar1 + 0x220) + 8)) {
  case 4:
    pcVar2 = (char *)(iVar1 + 0x85a);
    break;
  case 5:
    pcVar2 = (char *)(iVar1 + 0x85b);
    break;
  case 6:
    pcVar2 = (char *)(iVar1 + 0x85c);
    break;
  case 7:
    pcVar2 = (char *)(iVar1 + 0x866);
    break;
  case 8:
    pcVar2 = (char *)(iVar1 + 0x85d);
    break;
  case 9:
    pcVar2 = (char *)(iVar1 + 0x85e);
    break;
  case 10:
    pcVar2 = (char *)(iVar1 + 0x85f);
    break;
  case 0xb:
    pcVar2 = (char *)(iVar1 + 0x860);
    break;
  case 0xc:
    pcVar2 = (char *)(iVar1 + 0x867);
    break;
  case 0xd:
    pcVar2 = (char *)(iVar1 + 0x861);
    break;
  case 0xe:
    pcVar2 = (char *)(iVar1 + 0x862);
    break;
  case 0xf:
    pcVar2 = (char *)(iVar1 + 0x868);
    break;
  default:
    goto switchD_0040abbf_default;
  }
  if ((pcVar2 != (char *)0x0) && (*pcVar2 == '\0')) {
    Sound_PlayChannel(*(int *)(iVar1 + 0x518));
    *pcVar2 = '\x01';
    *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2a8) + 4) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2b0) + 4);
    Ball_SetName(*(void **)(param_1 + 0x29d0),"ARENA UNLOCKED!");
  }
switchD_0040abbf_default:
  return;
}
