
void __fastcall AthenaList_WriteDword(int param_1,uint param_2)

{
  undefined4 *puVar1;
  int *in_EAX;
  int iVar2;
  int iVar3;
  
  if (param_1 < in_EAX[3]) {
    puVar1 = (undefined4 *)*in_EAX;
    in_EAX[1] = (int)puVar1;
    in_EAX[3] = 0;
    in_EAX[4] = puVar1[2];
    in_EAX[2] = puVar1[1] + *(int *)*puVar1;
  }
  iVar3 = 4;
  do {
    iVar2 = AthenaList_IterateNext();
    *(char *)((*(int *)(iVar2 + 8) - *(int *)(iVar2 + 0xc)) + param_1) = (char)param_2;
    param_2 = param_2 >> 8;
    param_1 = param_1 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}

