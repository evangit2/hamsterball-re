
undefined8 __cdecl FileStream_SeekRead(undefined4 *param_1,uint param_2,int param_3)

{
  uint uVar1;
  undefined8 uVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  int *unaff_ESI;
  bool bVar6;
  
  if (param_3 < 0) goto LAB_004778a2;
  if ((0 < param_3) || (param_2 != 0)) {
    bVar6 = CARRY4(param_2,unaff_ESI[2]);
    param_2 = param_2 + unaff_ESI[2];
    param_3 = param_3 + unaff_ESI[3] + (uint)bVar6;
  }
  while( true ) {
    while( true ) {
      if (((-1 < param_3) && ((0 < param_3 || (param_2 != 0)))) &&
         ((param_3 < unaff_ESI[3] || ((param_3 <= unaff_ESI[3] && (param_2 <= (uint)unaff_ESI[2]))))
         )) {
        return 0xffffffffffffffff;
      }
LAB_004778a2:
      uVar3 = Ogg_ParsePage(unaff_ESI[6],param_1);
      if (-1 < (int)uVar3) break;
      uVar1 = unaff_ESI[2];
      unaff_ESI[2] = uVar1 - uVar3;
      unaff_ESI[3] = (unaff_ESI[3] - ((int)uVar3 >> 0x1f)) - (uint)(uVar1 < uVar3);
    }
    if (uVar3 != 0) {
      uVar1 = unaff_ESI[2];
      uVar2 = *(undefined8 *)(unaff_ESI + 2);
      unaff_ESI[3] = ((int)uVar3 >> 0x1f) + unaff_ESI[3] + (uint)CARRY4(uVar3,uVar1);
      unaff_ESI[2] = uVar3 + uVar1;
      return uVar2;
    }
    if (param_2 == 0 && param_3 == 0) break;
    piVar4 = __errno();
    *piVar4 = 0;
    if (*unaff_ESI == 0) {
      return 0xfffffffffffffffe;
    }
    iVar5 = StreamWriter_ReserveSpace(unaff_ESI[6],0x400);
    iVar5 = (*(code *)unaff_ESI[0x46])(iVar5,1,0x400,*unaff_ESI);
    if (0 < iVar5) {
      StreamWriter_CommitSize(unaff_ESI[6],iVar5);
    }
    if (iVar5 == 0) {
      piVar4 = __errno();
      if (*piVar4 != 0) {
        return 0xffffffffffffff80;
      }
      return 0xfffffffffffffffe;
    }
    if (iVar5 < 0) {
      return 0xffffffffffffff80;
    }
  }
  return 0xffffffffffffffff;
}

