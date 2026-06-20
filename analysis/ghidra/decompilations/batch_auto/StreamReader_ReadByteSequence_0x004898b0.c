
void __fastcall StreamReader_ReadByteSequence(int param_1)

{
  undefined1 *puVar1;
  undefined4 *puVar2;
  int *in_EAX;
  int iVar3;
  int iVar4;
  undefined1 auStack_8 [8];
  
  iVar4 = 0;
  if (param_1 < in_EAX[3]) {
    puVar2 = (undefined4 *)*in_EAX;
    in_EAX[1] = (int)puVar2;
    in_EAX[3] = 0;
    in_EAX[4] = puVar2[2];
    in_EAX[2] = puVar2[1] + *(int *)*puVar2;
  }
  do {
    iVar3 = AthenaList_IterateNext();
    puVar1 = (undefined1 *)((*(int *)(iVar3 + 8) - *(int *)(iVar3 + 0xc)) + param_1);
    param_1 = param_1 + 1;
    auStack_8[iVar4] = *puVar1;
    iVar4 = iVar4 + 1;
  } while (iVar4 < 7);
  AthenaList_IterateNext();
  iVar4 = 6;
  do {
    iVar4 = iVar4 + -1;
  } while (-1 < iVar4);
  return;
}

