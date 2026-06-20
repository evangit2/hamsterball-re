
void DCT_Stage16(void)

{
  uint uVar1;
  int iVar2;
  int *unaff_ESI;
  int iVar3;
  longlong lVar4;
  
  iVar3 = *unaff_ESI - unaff_ESI[8];
  unaff_ESI[8] = unaff_ESI[8] + *unaff_ESI;
  iVar2 = unaff_ESI[1] - unaff_ESI[9];
  uVar1 = iVar2 + iVar3;
  unaff_ESI[9] = unaff_ESI[9] + unaff_ESI[1];
  lVar4 = __allmul(uVar1,(int)uVar1 >> 0x1f,0x5a82799a,0);
  *unaff_ESI = (int)((ulonglong)lVar4 >> 0x20) * 2;
  uVar1 = iVar2 - iVar3;
  lVar4 = __allmul(uVar1,(int)uVar1 >> 0x1f,0x5a82799a,0);
  unaff_ESI[1] = (int)((ulonglong)lVar4 >> 0x20) * 2;
  iVar2 = unaff_ESI[10];
  unaff_ESI[10] = unaff_ESI[2] + iVar2;
  iVar3 = unaff_ESI[3];
  unaff_ESI[3] = iVar2 - unaff_ESI[2];
  unaff_ESI[2] = iVar3 - unaff_ESI[0xb];
  unaff_ESI[0xb] = unaff_ESI[0xb] + iVar3;
  iVar3 = unaff_ESI[0xc] - unaff_ESI[4];
  unaff_ESI[0xc] = unaff_ESI[4] + unaff_ESI[0xc];
  iVar2 = unaff_ESI[0xd] - unaff_ESI[5];
  uVar1 = iVar3 - iVar2;
  unaff_ESI[0xd] = unaff_ESI[5] + unaff_ESI[0xd];
  lVar4 = __allmul(uVar1,(int)uVar1 >> 0x1f,0x5a82799a,0);
  unaff_ESI[4] = (int)((ulonglong)lVar4 >> 0x20) * 2;
  uVar1 = iVar2 + iVar3;
  lVar4 = __allmul(uVar1,(int)uVar1 >> 0x1f,0x5a82799a,0);
  iVar2 = unaff_ESI[0xe];
  unaff_ESI[5] = (int)((ulonglong)lVar4 >> 0x20) * 2;
  unaff_ESI[0xe] = unaff_ESI[6] + iVar2;
  iVar3 = unaff_ESI[0xf];
  unaff_ESI[6] = iVar2 - unaff_ESI[6];
  unaff_ESI[0xf] = unaff_ESI[7] + iVar3;
  unaff_ESI[7] = iVar3 - unaff_ESI[7];
  DCT_Butterfly8();
  DCT_Butterfly8();
  return;
}

