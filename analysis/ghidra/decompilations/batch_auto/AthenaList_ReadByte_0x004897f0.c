
undefined1 __fastcall AthenaList_ReadByte(int param_1)

{
  undefined4 *puVar1;
  int *in_EAX;
  int iVar2;
  
  if (param_1 < in_EAX[3]) {
    puVar1 = (undefined4 *)*in_EAX;
    in_EAX[1] = (int)puVar1;
    in_EAX[3] = 0;
    in_EAX[4] = puVar1[2];
    in_EAX[2] = puVar1[1] + *(int *)*puVar1;
  }
  iVar2 = AthenaList_IterateNext();
  return *(undefined1 *)((*(int *)(iVar2 + 8) - *(int *)(iVar2 + 0xc)) + param_1);
}

