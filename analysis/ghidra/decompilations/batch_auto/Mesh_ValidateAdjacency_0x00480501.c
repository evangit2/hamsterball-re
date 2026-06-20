
undefined1 __thiscall Mesh_ValidateAdjacency(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  uint *puVar11;
  int *piVar12;
  uint local_14;
  uint *local_10;
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
    local_14 = 0;
    if (*(int *)((int)this + 0x58) != 0) {
      puVar11 = *(uint **)((int)this + 0x44);
      piVar12 = (int *)(local_c + 4);
      iVar2 = (int)puVar11 - local_c;
      do {
        if (piVar12[-1] == -1) {
          if (((*piVar12 != -1) || (piVar12[1] != -1)) ||
             (((*(byte *)((int)this + 0xc) & 1) != 0 &&
              (((*puVar11 != 0xffffffff || (*(int *)(iVar2 + (int)piVar12) != -1)) ||
               (puVar11[2] != 0xffffffff)))))) goto LAB_004806bd;
        }
        else if ((*(byte *)((int)this + 0xc) & 1) != 0) {
          uVar8 = 0;
          local_10 = puVar11;
          do {
            uVar3 = *(uint *)(((int)(piVar12 + -1) - (int)puVar11) + (int)local_10);
            uVar8 = uVar8 + 1;
            uVar4 = piVar12[uVar8 % 3 - 1];
            if ((*(uint *)((int)this + 0x30) <= uVar3) || (*(uint *)((int)this + 0x30) <= uVar4))
            goto LAB_004806bd;
            uVar9 = *local_10;
            if (uVar9 != 0xffffffff) {
              if (*(uint *)((int)this + 0x58) <= uVar9) goto LAB_004806bd;
              iVar1 = uVar9 * 0xc + local_c;
              uVar9 = FindInSmallIntArray(uVar9 * 0xc + *(int *)((int)this + 0x44),local_14);
              if (2 < uVar9) goto LAB_004806bd;
              if ((*(byte *)((int)this + 0xc) & 2) != 0) {
                iVar7 = *(int *)((int)this + 0x38);
                iVar5 = *(int *)(iVar7 + *(int *)(iVar1 + uVar9 * 4) * 4);
                iVar1 = *(int *)(iVar1 + ((uVar9 + 1) % 3) * 4);
                iVar6 = *(int *)(iVar7 + uVar3 * 4);
                if (((iVar6 != iVar5) ||
                    (*(int *)(iVar7 + uVar4 * 4) != *(int *)(iVar7 + iVar1 * 4))) &&
                   ((*(int *)(iVar7 + uVar4 * 4) != iVar5 || (iVar6 != *(int *)(iVar7 + iVar1 * 4)))
                   )) goto LAB_004806bd;
              }
            }
            local_10 = local_10 + 1;
          } while (uVar8 < 3);
        }
        local_14 = local_14 + 1;
        piVar12 = piVar12 + 3;
        puVar11 = puVar11 + 3;
      } while (local_14 < *(uint *)((int)this + 0x58));
    }
    if ((((*(byte *)((int)this + 0xc) & 0x10) == 0) || (*(int *)((int)this + 0x50) == 0)) ||
       (uVar10 = Mesh_ValidateAttributes((int)this), (char)uVar10 != '\0')) {
      local_5 = 1;
    }
  }
LAB_004806bd:
  if ((local_c != 0) && (param_1 == 0)) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  return local_5;
}

