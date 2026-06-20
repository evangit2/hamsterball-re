
undefined4 __fastcall AthenaList_ReadDword(int param_1)

{
  undefined1 uVar1;
  undefined1 uVar2;
  undefined1 uVar3;
  undefined4 *puVar4;
  int *in_EAX;
  int iVar5;
  
  if (param_1 < in_EAX[3]) {
    puVar4 = (undefined4 *)*in_EAX;
    in_EAX[1] = (int)puVar4;
    in_EAX[3] = 0;
    in_EAX[4] = puVar4[2];
    in_EAX[2] = puVar4[1] + *(int *)*puVar4;
  }
  iVar5 = AthenaList_IterateNext();
  uVar1 = *(undefined1 *)((*(int *)(iVar5 + 8) - *(int *)(iVar5 + 0xc)) + param_1);
  iVar5 = AthenaList_IterateNext();
  uVar2 = *(undefined1 *)((*(int *)(iVar5 + 8) - *(int *)(iVar5 + 0xc)) + param_1 + 1);
  iVar5 = AthenaList_IterateNext();
  uVar3 = *(undefined1 *)((*(int *)(iVar5 + 8) - *(int *)(iVar5 + 0xc)) + param_1 + 2);
  iVar5 = AthenaList_IterateNext();
  return CONCAT13(*(undefined1 *)((*(int *)(iVar5 + 8) - *(int *)(iVar5 + 0xc)) + param_1 + 3),
                  CONCAT12(uVar3,CONCAT11(uVar2,uVar1)));
}

