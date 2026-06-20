
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Sound_PlayChannel(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(param_1 + 4);
  if (((iVar1 != 0) && (*(int *)(iVar1 + 0x84c) != 0)) &&
     (*(float *)(iVar1 + 0x838) != _DAT_004cf368)) {
    iVar1 = *(int *)(param_1 + 0x10);
    if (iVar1 < *(int *)(param_1 + 0xc)) {
      iVar2 = *(int *)(*(int *)(param_1 + 0x414) + iVar1 * 4);
      *(int *)(param_1 + 0x10) = iVar1 + 1;
      if (iVar2 != 0) goto LAB_004597fd;
    }
    iVar2 = AthenaList_SetIndex((void *)(param_1 + 8),0);
    if (iVar2 != 0) {
LAB_004597fd:
      Sound_StartSample(iVar2);
      return;
    }
  }
  return;
}

