
undefined4 __thiscall MeshData_GenerateVertexRemap(void *this,int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  undefined4 local_8;
  
  uVar5 = 0;
  uVar4 = 0;
  if (*(int *)((int)this + 0x30) != 0) {
    do {
      *(undefined4 *)(param_2 + uVar4 * 4) = 0xffffffff;
      uVar4 = uVar4 + 1;
    } while (uVar4 < *(uint *)((int)this + 0x30));
  }
  local_8 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    do {
      iVar2 = *(int *)(param_1 + uVar5 * 4);
      if (iVar2 != -1) {
        iVar3 = *(int *)((int)this + 0x40);
        uVar4 = 0;
        do {
          piVar1 = (int *)(param_2 + *(int *)(iVar3 + iVar2 * 0xc + uVar4 * 4) * 4);
          if (*piVar1 == -1) {
            *piVar1 = local_8;
            local_8 = local_8 + 1;
          }
          uVar4 = uVar4 + 1;
        } while (uVar4 < 3);
      }
      uVar5 = uVar5 + 1;
    } while (uVar5 < *(uint *)((int)this + 0x58));
  }
  return 0;
}

