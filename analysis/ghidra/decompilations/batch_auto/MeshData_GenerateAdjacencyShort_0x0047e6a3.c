
undefined4 __thiscall MeshData_GenerateAdjacencyShort(void *this,int param_1,int param_2)

{
  uint *puVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  ushort *puVar7;
  int local_c;
  
  iVar4 = param_2;
  uVar6 = 0;
  if (*(int *)((int)this + 0x30) != 0) {
    uVar5 = 0;
    do {
      uVar6 = uVar6 + 1;
      *(undefined4 *)(param_2 + uVar5 * 4) = 0xffff;
      uVar5 = uVar6 & 0xffff;
    } while (uVar5 < *(uint *)((int)this + 0x30));
  }
  param_2 = 0;
  uVar2 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    uVar6 = 0;
    do {
      uVar3 = *(ushort *)(param_1 + uVar6 * 4);
      if (uVar3 != 0xffff) {
        puVar7 = (ushort *)(*(int *)((int)this + 0x40) + (uint)uVar3 * 6);
        local_c = 3;
        do {
          puVar1 = (uint *)(iVar4 + (uint)*puVar7 * 4);
          if (*puVar1 == 0xffff) {
            uVar6 = param_2 & 0xffff;
            param_2 = param_2 + 1;
            *puVar1 = uVar6;
          }
          puVar7 = puVar7 + 1;
          local_c = local_c + -1;
        } while (local_c != 0);
      }
      uVar2 = uVar2 + 1;
      uVar6 = (uint)uVar2;
    } while (uVar6 < *(uint *)((int)this + 0x58));
  }
  return 0;
}

