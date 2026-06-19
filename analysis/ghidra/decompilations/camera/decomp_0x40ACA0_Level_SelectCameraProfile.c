/* Ghidra Decompilation
 * Function: Level_SelectCameraProfile
 * Address: 0x40ACA0
 * Decompiled: 2026-06-19
 */


void __fastcall Level_SelectCameraProfile(int param_1)

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
    goto switchD_0040acbb_default;
  }
  if (pcVar2 != (char *)0x0) {
    if (*pcVar2 == '\0') {
      *(undefined4 *)(*(int *)(iVar1 + 0x2a8) + 4) = *(undefined4 *)(*(int *)(iVar1 + 0x2ac) + 4);
      return;
    }
    *(undefined4 *)(*(int *)(iVar1 + 0x2a8) + 4) = *(undefined4 *)(*(int *)(iVar1 + 0x2b0) + 4);
  }
switchD_0040acbb_default:
  return;
}

