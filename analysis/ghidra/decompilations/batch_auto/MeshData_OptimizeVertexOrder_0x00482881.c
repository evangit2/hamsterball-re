
undefined4 __thiscall MeshData_OptimizeVertexOrder(void *this,int param_1,int param_2)

{
  ushort *puVar1;
  int *piVar2;
  ushort uVar3;
  undefined4 *_Memory;
  uint uVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 local_10;
  uint local_8;
  
  local_10 = 0;
  uVar4 = *(uint *)((int)this + 0x30);
  if (*(uint *)((int)this + 0x30) <= *(uint *)((int)this + 0x58)) {
    uVar4 = *(uint *)((int)this + 0x58);
  }
  _Memory = operator_new(uVar4);
  if (_Memory == (undefined4 *)0x0) {
    local_10 = 0x8007000e;
  }
  else {
    if (((*(byte *)((int)this + 0xc) & 2) != 0) && (uVar4 = 0, *(int *)((int)this + 0x30) != 0)) {
      do {
        if (*(int *)(param_1 + uVar4 * 4) != 0xffff) {
          puVar1 = (ushort *)(*(int *)((int)this + 0x38) + uVar4 * 2);
          uVar3 = *puVar1;
          if (*(int *)(param_1 + (uint)uVar3 * 4) == 0xffff) {
            *puVar1 = (ushort)uVar4;
            uVar5 = uVar4;
            while (uVar5 = uVar5 + 1, uVar5 < *(uint *)((int)this + 0x30)) {
              puVar1 = (ushort *)(*(int *)((int)this + 0x38) + uVar5 * 2);
              if (*puVar1 == uVar3) {
                *puVar1 = (ushort)uVar4;
              }
            }
          }
        }
        uVar4 = uVar4 + 1;
      } while (uVar4 < *(uint *)((int)this + 0x30));
    }
    uVar4 = *(uint *)((int)this + 0x30);
    puVar7 = _Memory;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *puVar7 = 0;
      puVar7 = puVar7 + 1;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined1 *)puVar7 = 0;
      puVar7 = (undefined4 *)((int)puVar7 + 1);
    }
    local_8 = 0;
    if (*(int *)((int)this + 0x30) != 0) {
      do {
        if ((*(char *)(local_8 + (int)_Memory) == '\0') &&
           (uVar4 = *(uint *)(param_1 + local_8 * 4), uVar4 != 0xffff)) {
          while (uVar4 != local_8) {
            Mesh_SwapVertexData(this,local_8,uVar4,param_2);
            MeshData_UpdateAttrIndex(this,uVar4,param_1);
            *(undefined1 *)(uVar4 + (int)_Memory) = 1;
            uVar4 = *(uint *)(param_1 + uVar4 * 4);
            if ((uVar4 == 0xffff) || (*(char *)(uVar4 + (int)_Memory) != '\0')) goto LAB_00482985;
          }
          MeshData_UpdateAttrIndex(this,local_8,param_1);
        }
LAB_00482985:
        local_8 = local_8 + 1;
      } while (local_8 < *(uint *)((int)this + 0x30));
    }
    uVar4 = 0;
    iVar6 = 0;
    if (*(int *)((int)this + 0x30) != 0) {
      do {
        piVar2 = (int *)(param_1 + uVar4 * 4);
        if (*piVar2 == 0xffff) {
          *piVar2 = -1;
        }
        else {
          iVar6 = iVar6 + 1;
        }
        uVar4 = uVar4 + 1;
      } while (uVar4 < *(uint *)((int)this + 0x30));
    }
    *(int *)((int)this + 0x30) = iVar6;
  }
  _free(_Memory);
  return local_10;
}

