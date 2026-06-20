
undefined4 __thiscall MeshData_GenerateAdjacency(void *this,int param_1,int param_2,int param_3)

{
  uint uVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  uint local_c;
  int local_8;
  
  uVar3 = 0;
  uVar1 = 0;
  if (*(int *)((int)this + 0x30) != 0) {
    do {
      *(undefined4 *)(param_3 + uVar1 * 4) = 0xffffffff;
      uVar1 = uVar1 + 1;
    } while (uVar1 < *(uint *)((int)this + 0x30));
  }
  uVar1 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    do {
      *(uint *)(param_2 + uVar1 * 4) = uVar1;
      uVar1 = uVar1 + 1;
    } while (uVar1 < *(uint *)((int)this + 0x58));
  }
  local_c = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    local_8 = 0;
    do {
      piVar2 = (int *)(*(int *)((int)this + 0x40) + local_8);
      if (*piVar2 == -1) {
        *(undefined4 *)(param_1 + uVar3 * 4) = 0xffffffff;
      }
      else {
        *(uint *)(param_1 + uVar3 * 4) = local_c;
        *(uint *)(param_2 + local_c * 4) = uVar3;
        local_c = local_c + 1;
        *(undefined4 *)(param_3 + *piVar2 * 4) = 1;
        *(undefined4 *)(param_3 + piVar2[1] * 4) = 1;
        *(undefined4 *)(param_3 + piVar2[2] * 4) = 1;
      }
      local_8 = local_8 + 0xc;
      uVar3 = uVar3 + 1;
    } while (uVar3 < *(uint *)((int)this + 0x58));
  }
  iVar4 = 0;
  for (; local_c < *(uint *)((int)this + 0x58); local_c = local_c + 1) {
    *(undefined4 *)(param_2 + local_c * 4) = 0xffffffff;
  }
  uVar1 = 0;
  if (*(int *)((int)this + 0x30) != 0) {
    do {
      if (*(int *)(param_3 + uVar1 * 4) != -1) {
        *(int *)(param_3 + uVar1 * 4) = iVar4;
        iVar4 = iVar4 + 1;
      }
      uVar1 = uVar1 + 1;
    } while (uVar1 < *(uint *)((int)this + 0x30));
  }
  return 0;
}

