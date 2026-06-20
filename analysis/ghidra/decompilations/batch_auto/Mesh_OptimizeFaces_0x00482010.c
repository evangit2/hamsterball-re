
undefined4 __thiscall Mesh_OptimizeFaces(void *this,int param_1,int param_2)

{
  uint *puVar1;
  undefined4 *_Memory;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 local_c;
  
  local_c = 0;
  uVar3 = *(uint *)((int)this + 0x30);
  if (*(uint *)((int)this + 0x30) <= *(uint *)((int)this + 0x58)) {
    uVar3 = *(uint *)((int)this + 0x58);
  }
  _Memory = operator_new(uVar3);
  if (_Memory == (undefined4 *)0x0) {
    local_c = 0x8007000e;
  }
  else {
    if (((*(byte *)((int)this + 0xc) & 2) != 0) && (uVar3 = 0, *(int *)((int)this + 0x30) != 0)) {
      do {
        if (*(int *)(param_1 + uVar3 * 4) != -1) {
          puVar1 = (uint *)(*(int *)((int)this + 0x38) + uVar3 * 4);
          uVar4 = *puVar1;
          if (*(int *)(param_1 + uVar4 * 4) == -1) {
            *puVar1 = uVar3;
            uVar2 = uVar3;
            while (uVar2 = uVar2 + 1, uVar2 < *(uint *)((int)this + 0x30)) {
              puVar1 = (uint *)(*(int *)((int)this + 0x38) + uVar2 * 4);
              if (*puVar1 == uVar4) {
                *puVar1 = uVar3;
              }
            }
          }
        }
        uVar3 = uVar3 + 1;
      } while (uVar3 < *(uint *)((int)this + 0x30));
    }
    uVar3 = *(uint *)((int)this + 0x30);
    puVar6 = _Memory;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar6 = 0;
      puVar6 = puVar6 + 1;
    }
    uVar4 = 0;
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *(undefined1 *)puVar6 = 0;
      puVar6 = (undefined4 *)((int)puVar6 + 1);
    }
    if (*(int *)((int)this + 0x30) != 0) {
      do {
        if ((*(char *)(uVar4 + (int)_Memory) == '\0') &&
           (uVar3 = *(uint *)(param_1 + uVar4 * 4), uVar3 != 0xffffffff)) {
          while (uVar3 != uVar4) {
            Mesh_SwapVertices32(this,uVar4,uVar3,param_2);
            Mesh_RemapVertexAttribute32(this,uVar3,param_1);
            *(undefined1 *)(uVar3 + (int)_Memory) = 1;
            uVar3 = *(uint *)(param_1 + uVar3 * 4);
            if ((uVar3 == 0xffffffff) || (*(char *)(uVar3 + (int)_Memory) != '\0'))
            goto LAB_004820ff;
          }
          Mesh_RemapVertexAttribute32(this,uVar4,param_1);
        }
LAB_004820ff:
        uVar4 = uVar4 + 1;
      } while (uVar4 < *(uint *)((int)this + 0x30));
    }
    uVar3 = 0;
    iVar5 = 0;
    if (*(int *)((int)this + 0x30) != 0) {
      do {
        if (*(int *)(param_1 + uVar3 * 4) == -1) {
          *(undefined4 *)(param_1 + uVar3 * 4) = 0xffffffff;
        }
        else {
          iVar5 = iVar5 + 1;
        }
        uVar3 = uVar3 + 1;
      } while (uVar3 < *(uint *)((int)this + 0x30));
    }
    *(int *)((int)this + 0x30) = iVar5;
  }
  _free(_Memory);
  return local_c;
}

