
undefined4 __thiscall Mesh_OptimizeVertices(void *this,int param_1,int param_2)

{
  int *piVar1;
  undefined4 *_Memory;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
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
    uVar3 = *(uint *)((int)this + 0x58);
    puVar5 = _Memory;
    for (uVar2 = uVar3 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar5 = 0;
      puVar5 = puVar5 + 1;
    }
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *(undefined1 *)puVar5 = 0;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
    }
    uVar3 = 0;
    if (*(int *)((int)this + 0x58) != 0) {
      do {
        if ((*(char *)(uVar3 + (int)_Memory) == '\0') &&
           (uVar2 = *(uint *)(param_1 + uVar3 * 4), uVar2 != 0xffffffff)) {
          while (uVar2 != uVar3) {
            Mesh_SwapVertices(this,uVar3,uVar2);
            Mesh_RemapVertexIndices(this,uVar2,param_2,param_1);
            *(undefined1 *)(uVar2 + (int)_Memory) = 1;
            uVar2 = *(uint *)(param_1 + uVar2 * 4);
            if ((uVar2 == 0xffffffff) || (*(char *)(uVar2 + (int)_Memory) != '\0'))
            goto LAB_004821dc;
          }
          Mesh_RemapVertexIndices(this,uVar3,param_2,param_1);
        }
LAB_004821dc:
        uVar3 = uVar3 + 1;
      } while (uVar3 < *(uint *)((int)this + 0x58));
    }
    uVar3 = 0;
    iVar4 = 0;
    if (*(int *)((int)this + 0x58) != 0) {
      do {
        piVar1 = (int *)(param_1 + uVar3 * 4);
        if (*piVar1 == -1) {
          *piVar1 = -1;
        }
        else {
          iVar4 = iVar4 + 1;
        }
        uVar3 = uVar3 + 1;
      } while (uVar3 < *(uint *)((int)this + 0x58));
    }
    *(int *)((int)this + 0x58) = iVar4;
  }
  _free(_Memory);
  return local_c;
}

