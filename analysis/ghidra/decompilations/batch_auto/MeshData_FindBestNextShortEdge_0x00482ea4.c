
undefined4 __thiscall MeshData_FindBestNextShortEdge(void *this,ushort param_1)

{
  char *pcVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  ushort uVar5;
  ushort uVar6;
  ushort *puVar7;
  int iVar8;
  ushort *puVar9;
  ushort *puVar10;
  ushort *local_1c;
  int local_18;
  ushort *local_10;
  ushort *local_8;
  
  uVar2 = 0;
  local_10 = (ushort *)(*(int *)((int)this + 4) + (uint)param_1 * 6);
  uVar5 = 3;
  uVar4 = 0xffff;
  do {
    uVar3 = *local_10;
    puVar7 = local_10;
    if (uVar3 != 0xffff) {
      iVar8 = (uint)uVar3 - (uint)*(ushort *)((int)this + 0x14);
      puVar7 = (ushort *)(iVar8 * 3);
      pcVar1 = (char *)(*(int *)((int)this + 0x10) + iVar8 * 6);
      if (*pcVar1 == '\0') {
        uVar6 = (ushort)(byte)pcVar1[1];
        puVar7 = (ushort *)0xffff;
        local_8 = (ushort *)0xffff;
        puVar10 = (ushort *)(*(int *)((int)this + 4) + (uint)uVar3 * 6);
        local_18 = 3;
        do {
          if (((*puVar10 != 0xffff) &&
              (pcVar1 = (char *)(*(int *)((int)this + 0x10) +
                                ((uint)*puVar10 - (uint)*(ushort *)((int)this + 0x14)) * 6),
              puVar7 = local_8, *pcVar1 == '\0')) &&
             (puVar9 = (ushort *)CONCAT22((short)((uint)pcVar1 >> 0x10),(ushort)(byte)pcVar1[1]),
             (ushort)(byte)pcVar1[1] < (ushort)local_8)) {
            puVar7 = puVar9;
            local_8 = puVar9;
          }
          puVar10 = puVar10 + 1;
          local_18 = local_18 + -1;
        } while (local_18 != 0);
        if ((short)puVar7 == -1) {
          puVar7 = (ushort *)0x0;
        }
        if ((uVar6 < uVar4) ||
           ((uVar6 == uVar4 && (uVar6 = uVar4, (ushort)puVar7 < (ushort)local_1c)))) {
          local_1c = puVar7;
          uVar4 = uVar6;
          uVar5 = uVar2;
        }
      }
    }
    uVar2 = uVar2 + 1;
    local_10 = local_10 + 1;
  } while (uVar2 < 3);
  return CONCAT22((short)((uint)puVar7 >> 0x10),uVar5);
}

