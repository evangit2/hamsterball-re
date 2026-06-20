
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

void __thiscall Mesh_SwapVertices32(void *this,int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  
  uVar5 = *(uint *)((int)this + 0x2c);
  iVar2 = -(uVar5 + 3 & 0xfffffffc);
  puVar6 = (undefined4 *)(uVar5 * param_2 + param_3);
  puVar3 = (undefined4 *)(uVar5 * param_1 + param_3);
  puVar7 = puVar6;
  puVar8 = (undefined4 *)(&stack0xffffffec + iVar2);
  for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar8 = *puVar7;
    puVar7 = puVar7 + 1;
    puVar8 = puVar8 + 1;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)puVar8 = *(undefined1 *)puVar7;
    puVar7 = (undefined4 *)((int)puVar7 + 1);
    puVar8 = (undefined4 *)((int)puVar8 + 1);
  }
  uVar5 = *(uint *)((int)this + 0x2c);
  puVar7 = puVar3;
  for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar6 = *puVar7;
    puVar7 = puVar7 + 1;
    puVar6 = puVar6 + 1;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)puVar6 = *(undefined1 *)puVar7;
    puVar7 = (undefined4 *)((int)puVar7 + 1);
    puVar6 = (undefined4 *)((int)puVar6 + 1);
  }
  uVar5 = *(uint *)((int)this + 0x2c);
  puVar7 = (undefined4 *)(&stack0xffffffec + iVar2);
  for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar3 = *puVar7;
    puVar7 = puVar7 + 1;
    puVar3 = puVar3 + 1;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)puVar3 = *(undefined1 *)puVar7;
    puVar7 = (undefined4 *)((int)puVar7 + 1);
    puVar3 = (undefined4 *)((int)puVar3 + 1);
  }
  if ((*(byte *)((int)this + 0xc) & 2) != 0) {
    puVar7 = (undefined4 *)(*(int *)((int)this + 0x38) + param_2 * 4);
    uVar1 = *puVar7;
    *puVar7 = *(undefined4 *)(param_1 * 4 + *(int *)((int)this + 0x38));
    *(undefined4 *)(param_1 * 4 + *(int *)((int)this + 0x38)) = uVar1;
  }
  return;
}

