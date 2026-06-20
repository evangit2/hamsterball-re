
void __cdecl EH_MediaParser(int *param_1)

{
  int iVar1;
  int *unaff_retaddr;
  undefined4 uVar2;
  
  (**(code **)(*param_1 + 8))(param_1);
  Audio_CodecReleaseHandle((int)param_1);
  uVar2 = 1;
  CRT_FlsAlloc(1);
  iVar1 = *unaff_retaddr;
  if ((int)param_1 < 0) {
    if ((*(int *)(iVar1 + 0x6c) == 0) || (2 < *(int *)(iVar1 + 0x68))) {
      (**(code **)(iVar1 + 8))(unaff_retaddr);
    }
    *(int *)(iVar1 + 0x6c) = *(int *)(iVar1 + 0x6c) + 1;
    return;
  }
  if ((int)param_1 <= *(int *)(iVar1 + 0x68)) {
    (**(code **)(iVar1 + 8))(unaff_retaddr,uVar2);
  }
  return;
}

