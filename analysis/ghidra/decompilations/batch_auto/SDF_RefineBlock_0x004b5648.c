
void __cdecl SDF_RefineBlock(int param_1,int param_2,int param_3)

{
  int iVar1;
  int in_EAX;
  int iVar2;
  short *psVar3;
  int *piVar4;
  byte *pbVar5;
  int iVar6;
  undefined1 local_18c [256];
  byte local_8c [128];
  int local_c;
  int local_8;
  
  iVar1 = param_3 >> 2;
  local_8 = *(int *)(*(int *)(param_1 + 0x1a4) + 0x18);
  local_c = iVar1 * 0x20 + 4;
  iVar6 = (param_2 >> 2) * 0x20 + 4;
  iVar2 = SDF_FindClosestPalette(iVar6,local_c,(int)local_18c);
  SDF_FindClosestPalette4x8x8
            (param_1,iVar6,(in_EAX >> 3) * 0x20 + 2,local_c,iVar2,(int)local_18c,local_8c);
  pbVar5 = local_8c;
  piVar4 = (int *)(local_8 + (param_2 >> 2) * 0x10);
  param_2 = 4;
  do {
    param_3 = 8;
    iVar2 = ((in_EAX >> 3) * 0x100 + iVar1 * 4) * 2;
    do {
      psVar3 = (short *)(*piVar4 + iVar2);
      iVar6 = 4;
      do {
        *psVar3 = *pbVar5 + 1;
        psVar3 = psVar3 + 1;
        pbVar5 = pbVar5 + 1;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
      iVar2 = iVar2 + 0x40;
      param_3 = param_3 + -1;
    } while (param_3 != 0);
    piVar4 = piVar4 + 1;
    param_2 = param_2 + -1;
  } while (param_2 != 0);
  return;
}

