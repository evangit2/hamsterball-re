
void Ogg_CalculateCRC(void)

{
  int iVar1;
  undefined4 *puVar2;
  int *in_EAX;
  uint uVar3;
  uint uVar4;
  int unaff_EDI;
  
  *(undefined4 *)(unaff_EDI + 0x48) = 0;
  if (*(int *)(unaff_EDI + 0x44) < *(int *)(unaff_EDI + 0x30)) {
    while( true ) {
      iVar1 = *(int *)(unaff_EDI + 0x44) + 0x1b;
      *(int *)(unaff_EDI + 0x44) = *(int *)(unaff_EDI + 0x44) + 1;
      if (iVar1 < in_EAX[3]) {
        puVar2 = (undefined4 *)*in_EAX;
        in_EAX[1] = (int)puVar2;
        in_EAX[3] = 0;
        in_EAX[4] = puVar2[2];
        in_EAX[2] = puVar2[1] + *(int *)*puVar2;
      }
      in_EAX = (int *)AthenaList_IterateNext();
      uVar3 = (uint)*(byte *)((in_EAX[2] - in_EAX[3]) + iVar1);
      uVar4 = *(int *)(unaff_EDI + 0x48) + uVar3;
      *(uint *)(unaff_EDI + 0x48) = uVar4;
      if (uVar3 < 0xff) break;
      if (*(int *)(unaff_EDI + 0x30) <= *(int *)(unaff_EDI + 0x44)) {
        return;
      }
    }
    *(uint *)(unaff_EDI + 0x48) = uVar4 | 0x80000000;
    *(undefined4 *)(unaff_EDI + 0x40) = 1;
  }
  return;
}

