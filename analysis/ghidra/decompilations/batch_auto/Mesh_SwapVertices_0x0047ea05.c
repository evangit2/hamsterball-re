
void __thiscall Mesh_SwapVertices(void *this,int param_1,int param_2)

{
  undefined2 *puVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  int local_8;
  
  puVar8 = (undefined4 *)(*(int *)((int)this + 0x40) + param_1 * 0xc);
  iVar4 = (*(int *)((int)this + 0x40) + param_2 * 0xc) - (int)puVar8;
  iVar5 = (*(int *)((int)this + 0x44) + param_2 * 0xc) - (int)puVar8;
  iVar6 = (param_1 * 0xc + *(int *)((int)this + 0x44)) - (int)puVar8;
  local_8 = 3;
  do {
    puVar7 = (undefined4 *)(iVar4 + (int)puVar8);
    uVar3 = *puVar7;
    *puVar7 = *puVar8;
    *puVar8 = uVar3;
    if ((*(byte *)((int)this + 0xc) & 1) != 0) {
      puVar7 = (undefined4 *)(iVar5 + (int)puVar8);
      uVar3 = *puVar7;
      *puVar7 = *(undefined4 *)(iVar6 + (int)puVar8);
      *(undefined4 *)(iVar6 + (int)puVar8) = uVar3;
    }
    puVar8 = puVar8 + 1;
    local_8 = local_8 + -1;
  } while (local_8 != 0);
  if ((*(byte *)((int)this + 0xc) & 4) != 0) {
    puVar8 = (undefined4 *)(*(int *)((int)this + 0x48) + param_2 * 4);
    uVar3 = *puVar8;
    *puVar8 = *(undefined4 *)(*(int *)((int)this + 0x48) + param_1 * 4);
    *(undefined4 *)(*(int *)((int)this + 0x48) + param_1 * 4) = uVar3;
  }
  if ((*(byte *)((int)this + 0xc) & 8) != 0) {
    puVar1 = (undefined2 *)(*(int *)((int)this + 0x4c) + param_2 * 2);
    uVar2 = *puVar1;
    *puVar1 = *(undefined2 *)(param_1 * 2 + *(int *)((int)this + 0x4c));
    *(undefined2 *)(param_1 * 2 + *(int *)((int)this + 0x4c)) = uVar2;
  }
  return;
}

