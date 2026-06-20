
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

void __thiscall Mesh_SwapVertexData(void *this,int param_1,int param_2,int param_3)

{
  undefined2 *puVar1;
  undefined2 uVar2;
  int iVar3;
  undefined4 *puVar4;
  uint uVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  
  uVar6 = *(uint *)((int)this + 0x2c);
  iVar3 = -(uVar6 + 3 & 0xfffffffc);
  puVar7 = (undefined4 *)(uVar6 * param_2 + param_3);
  puVar4 = (undefined4 *)(uVar6 * param_1 + param_3);
  puVar8 = puVar7;
  puVar9 = (undefined4 *)(&stack0xffffffec + iVar3);
  for (uVar5 = uVar6 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar9 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar9 = puVar9 + 1;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined1 *)puVar9 = *(undefined1 *)puVar8;
    puVar8 = (undefined4 *)((int)puVar8 + 1);
    puVar9 = (undefined4 *)((int)puVar9 + 1);
  }
  uVar6 = *(uint *)((int)this + 0x2c);
  puVar8 = puVar4;
  for (uVar5 = uVar6 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar7 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar7 = puVar7 + 1;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined1 *)puVar7 = *(undefined1 *)puVar8;
    puVar8 = (undefined4 *)((int)puVar8 + 1);
    puVar7 = (undefined4 *)((int)puVar7 + 1);
  }
  uVar6 = *(uint *)((int)this + 0x2c);
  puVar8 = (undefined4 *)(&stack0xffffffec + iVar3);
  for (uVar5 = uVar6 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar4 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar4 = puVar4 + 1;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined1 *)puVar4 = *(undefined1 *)puVar8;
    puVar8 = (undefined4 *)((int)puVar8 + 1);
    puVar4 = (undefined4 *)((int)puVar4 + 1);
  }
  if ((*(byte *)((int)this + 0xc) & 2) != 0) {
    puVar1 = (undefined2 *)(*(int *)((int)this + 0x38) + param_2 * 2);
    uVar2 = *puVar1;
    *puVar1 = *(undefined2 *)(param_1 * 2 + *(int *)((int)this + 0x38));
    *(undefined2 *)(param_1 * 2 + *(int *)((int)this + 0x38)) = uVar2;
  }
  return;
}

