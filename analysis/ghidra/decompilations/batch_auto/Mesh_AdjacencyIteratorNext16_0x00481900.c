
undefined4 __fastcall Mesh_AdjacencyIteratorNext16(ushort *param_1)

{
  ushort uVar1;
  char cVar2;
  ushort uVar3;
  int iVar4;
  undefined2 uVar6;
  undefined3 extraout_var;
  uint uVar5;
  ushort uVar7;
  
  uVar1 = param_1[2];
  param_1[3] = param_1[4];
  iVar4 = (uint)uVar1 * 3 + (uint)param_1[4];
  uVar3 = *(ushort *)(*(int *)(*(int *)(param_1 + 6) + 0x44) + iVar4 * 2);
  uVar6 = (undefined2)((uint)iVar4 >> 0x10);
  param_1[2] = uVar3;
  uVar7 = uVar1;
  if (uVar3 != *param_1) {
    do {
      if (param_1[2] != 0xffff) {
        uVar3 = ShortArray_FindMatch3
                          (*(int *)(*(int *)(param_1 + 6) + 0x44) + (uint)param_1[2] * 6,uVar7);
        param_1[4] = uVar3;
        uVar5 = uVar3 + 1;
        if ((char)param_1[8] == '\0') {
          uVar5 = uVar3 + 2;
        }
        uVar6 = 0;
        param_1[4] = (ushort)((ulonglong)uVar5 % 3);
        goto LAB_004819b2;
      }
      if (((char)param_1[8] == '\0') || (*(char *)((int)param_1 + 0x11) != '\0')) goto LAB_004819b2;
      *(undefined1 *)(param_1 + 8) = 0;
      param_1[2] = *param_1;
      cVar2 = WideString_MatchSlot
                        ((ushort *)(*(int *)(*(int *)(param_1 + 6) + 0x40) + (uint)*param_1 * 6),
                         (uint)param_1[1]);
      param_1[4] = (ushort)((ulonglong)((CONCAT31(extraout_var,cVar2) & 0xffff) + 2) % 3);
      param_1[3] = (ushort)((ulonglong)(param_1[3] + 2) % 3);
      uVar7 = param_1[2];
      iVar4 = (uint)uVar7 * 3 + (uint)param_1[4];
      uVar3 = *(ushort *)(*(int *)(*(int *)(param_1 + 6) + 0x44) + iVar4 * 2);
      uVar6 = (undefined2)((uint)iVar4 >> 0x10);
      param_1[2] = uVar3;
    } while (uVar3 != *param_1);
  }
  param_1[2] = 0xffff;
LAB_004819b2:
  return CONCAT22(uVar6,uVar1);
}

