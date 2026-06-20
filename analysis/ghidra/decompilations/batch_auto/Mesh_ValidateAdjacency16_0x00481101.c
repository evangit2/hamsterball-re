
undefined1 __thiscall Mesh_ValidateAdjacency16(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  ushort uVar3;
  short sVar4;
  short sVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  undefined4 uVar9;
  uint uVar10;
  ushort *puVar11;
  uint uVar12;
  short *psVar13;
  uint local_18;
  ushort *local_10;
  int local_c;
  undefined1 local_5;
  
  local_5 = 0;
  iVar2 = param_1;
  iVar1 = param_1;
  if (((param_1 != 0) || (iVar2 = *(int *)((int)this + 0x40), iVar1 = iVar2, iVar2 != 0)) ||
     (iVar7 = Graphics_DrawIndexedPrimitive(this,&local_c,0x10), iVar2 = param_1, iVar1 = local_c,
     -1 < iVar7)) {
    local_c = iVar1;
    param_1 = iVar2;
    local_18 = 0;
    if (*(int *)((int)this + 0x58) != 0) {
      puVar11 = *(ushort **)((int)this + 0x44);
      psVar13 = (short *)(local_c + 2);
      iVar2 = (int)puVar11 - local_c;
      do {
        if (psVar13[-1] == -1) {
          if (((*psVar13 != -1) || (psVar13[1] != -1)) ||
             (((*(byte *)((int)this + 0xc) & 1) != 0 &&
              (((*puVar11 != 0xffff || (*(short *)(iVar2 + (int)psVar13) != -1)) ||
               (puVar11[2] != 0xffff)))))) goto LAB_004812ef;
        }
        else if ((*(byte *)((int)this + 0xc) & 1) != 0) {
          uVar6 = 0;
          local_10 = puVar11;
          do {
            uVar10 = (uint)*(ushort *)(((int)(psVar13 + -1) - (int)puVar11) + (int)local_10);
            uVar6 = uVar6 + 1;
            uVar12 = (uint)(ushort)psVar13[uVar6 % 3 - 1];
            if ((*(uint *)((int)this + 0x30) <= uVar10) || (*(uint *)((int)this + 0x30) <= uVar12))
            goto LAB_004812ef;
            uVar3 = *local_10;
            if (uVar3 != 0xffff) {
              if (*(uint *)((int)this + 0x58) <= (uint)uVar3) goto LAB_004812ef;
              iVar7 = (uint)uVar3 * 6;
              iVar1 = iVar7 + local_c;
              uVar8 = ShortArray_FindMatch3(iVar7 + *(int *)((int)this + 0x44),(short)local_18);
              if (2 < uVar8) goto LAB_004812ef;
              if ((*(byte *)((int)this + 0xc) & 2) != 0) {
                iVar7 = *(int *)((int)this + 0x38);
                sVar4 = *(short *)(iVar7 + (uint)*(ushort *)(iVar1 + uVar8 * 2) * 2);
                uVar8 = (uint)*(ushort *)(iVar1 + ((uVar8 + 1) % 3) * 2);
                sVar5 = *(short *)(iVar7 + uVar10 * 2);
                if (((sVar5 != sVar4) ||
                    (*(short *)(iVar7 + uVar12 * 2) != *(short *)(iVar7 + uVar8 * 2))) &&
                   ((*(short *)(iVar7 + uVar12 * 2) != sVar4 ||
                    (sVar5 != *(short *)(iVar7 + uVar8 * 2))))) goto LAB_004812ef;
              }
            }
            local_10 = local_10 + 1;
          } while (uVar6 < 3);
        }
        local_18 = local_18 + 1;
        psVar13 = psVar13 + 3;
        puVar11 = puVar11 + 3;
      } while (local_18 < *(uint *)((int)this + 0x58));
    }
    if ((((*(byte *)((int)this + 0xc) & 0x10) == 0) || (*(int *)((int)this + 0x50) == 0)) ||
       (uVar9 = Mesh_ValidateAttributes((int)this), (char)uVar9 != '\0')) {
      local_5 = 1;
    }
  }
LAB_004812ef:
  if ((local_c != 0) && (param_1 == 0)) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  return local_5;
}

