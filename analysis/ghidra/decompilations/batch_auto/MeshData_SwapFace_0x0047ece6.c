
void __thiscall MeshData_SwapFace(void *this,int param_1,int param_2)

{
  undefined4 *puVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined2 *puVar7;
  undefined2 *puVar8;
  int local_8;
  
  puVar8 = (undefined2 *)(*(int *)((int)this + 0x40) + param_1 * 6);
  iVar4 = (*(int *)((int)this + 0x40) + param_2 * 6) - (int)puVar8;
  iVar5 = (*(int *)((int)this + 0x44) + param_2 * 6) - (int)puVar8;
  iVar6 = (param_1 * 6 + *(int *)((int)this + 0x44)) - (int)puVar8;
  local_8 = 3;
  do {
    puVar7 = (undefined2 *)(iVar4 + (int)puVar8);
    uVar2 = *puVar7;
    *puVar7 = *puVar8;
    *puVar8 = uVar2;
    if ((*(byte *)((int)this + 0xc) & 1) != 0) {
      puVar7 = (undefined2 *)(iVar5 + (int)puVar8);
      uVar2 = *puVar7;
      *puVar7 = *(undefined2 *)(iVar6 + (int)puVar8);
      *(undefined2 *)(iVar6 + (int)puVar8) = uVar2;
    }
    puVar8 = puVar8 + 1;
    local_8 = local_8 + -1;
  } while (local_8 != 0);
  if ((*(byte *)((int)this + 0xc) & 4) != 0) {
    puVar1 = (undefined4 *)(*(int *)((int)this + 0x48) + param_2 * 4);
    uVar3 = *puVar1;
    *puVar1 = *(undefined4 *)(*(int *)((int)this + 0x48) + param_1 * 4);
    *(undefined4 *)(*(int *)((int)this + 0x48) + param_1 * 4) = uVar3;
  }
  if ((*(byte *)((int)this + 0xc) & 8) != 0) {
    puVar8 = (undefined2 *)(*(int *)((int)this + 0x4c) + param_2 * 2);
    uVar2 = *puVar8;
    *puVar8 = *(undefined2 *)(param_1 * 2 + *(int *)((int)this + 0x4c));
    *(undefined2 *)(param_1 * 2 + *(int *)((int)this + 0x4c)) = uVar2;
  }
  return;
}

