
uint __thiscall MeshData_FindBestNextEdge(void *this,uint param_1)

{
  char *pcVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint local_18;
  uint local_14;
  uint local_10;
  uint local_c;
  uint local_8;
  
  uVar4 = param_1;
  local_10 = 0xffffffff;
  param_1 = 0;
  local_14 = 3;
  do {
    iVar2 = *(int *)(*(int *)((int)this + 4) + uVar4 * 0xc + param_1 * 4);
    if (iVar2 != -1) {
      pcVar1 = (char *)(*(int *)((int)this + 0x18) + (iVar2 - *(int *)((int)this + 0x1c)) * 0xc);
      if (*pcVar1 == '\0') {
        uVar5 = (uint)(byte)pcVar1[1];
        uVar6 = 0xffffffff;
        local_8 = 0;
        local_c = 0xffffffff;
        do {
          iVar3 = *(int *)(*(int *)((int)this + 4) + iVar2 * 0xc + local_8 * 4);
          if (((iVar3 != -1) &&
              (pcVar1 = (char *)(*(int *)((int)this + 0x18) +
                                (iVar3 - *(int *)((int)this + 0x1c)) * 0xc), uVar6 = local_c,
              *pcVar1 == '\0')) && (uVar7 = (uint)(byte)pcVar1[1], uVar7 < local_c)) {
            uVar6 = uVar7;
            local_c = uVar7;
          }
          local_8 = local_8 + 1;
        } while (local_8 < 3);
        if (uVar6 == 0xffffffff) {
          uVar6 = 0;
        }
        if ((uVar5 < local_10) || ((uVar5 == local_10 && (uVar5 = local_10, uVar6 < local_18)))) {
          local_10 = uVar5;
          local_14 = param_1;
          local_18 = uVar6;
        }
      }
    }
    param_1 = param_1 + 1;
  } while (param_1 < 3);
  return local_14;
}

