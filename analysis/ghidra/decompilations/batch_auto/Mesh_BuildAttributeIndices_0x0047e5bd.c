
undefined4 __thiscall Mesh_BuildAttributeIndices(void *this,int param_1,int param_2,int param_3)

{
  ushort *puVar1;
  uint *puVar2;
  ushort uVar3;
  uint uVar4;
  uint uVar5;
  ushort uVar6;
  uint uVar7;
  
  uVar7 = 0;
  if (*(int *)((int)this + 0x30) != 0) {
    uVar4 = 0;
    do {
      uVar7 = uVar7 + 1;
      *(undefined4 *)(param_3 + uVar4 * 4) = 0xffff;
      uVar4 = uVar7 & 0xffff;
    } while (uVar4 < *(uint *)((int)this + 0x30));
  }
  uVar7 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    uVar4 = 0;
    do {
      uVar7 = uVar7 + 1;
      *(uint *)(param_2 + uVar4 * 4) = uVar4;
      uVar4 = uVar7 & 0xffff;
    } while (uVar4 < *(uint *)((int)this + 0x58));
  }
  uVar6 = 0;
  uVar3 = 0;
  if (*(int *)((int)this + 0x58) != 0) {
    uVar7 = 0;
    do {
      puVar1 = (ushort *)(*(int *)((int)this + 0x40) + uVar7 * 6);
      if (*puVar1 == 0xffff) {
        *(undefined4 *)(param_1 + uVar7 * 4) = 0xffff;
      }
      else {
        uVar4 = (uint)uVar6;
        uVar6 = uVar6 + 1;
        *(uint *)(param_1 + uVar7 * 4) = uVar4;
        *(uint *)(param_2 + uVar4 * 4) = uVar7;
        *(undefined4 *)(param_3 + (uint)*puVar1 * 4) = 1;
        *(undefined4 *)(param_3 + (uint)puVar1[1] * 4) = 1;
        *(undefined4 *)(param_3 + (uint)puVar1[2] * 4) = 1;
      }
      uVar3 = uVar3 + 1;
      uVar7 = (uint)uVar3;
    } while (uVar7 < *(uint *)((int)this + 0x58));
  }
  uVar7 = 0;
  for (; (uint)uVar6 < *(uint *)((int)this + 0x58); uVar6 = uVar6 + 1) {
    *(undefined4 *)(param_2 + (uint)uVar6 * 4) = 0xffff;
  }
  uVar4 = 0;
  if (*(int *)((int)this + 0x30) != 0) {
    uVar5 = 0;
    do {
      puVar2 = (uint *)(param_3 + uVar5 * 4);
      if (*puVar2 != 0xffff) {
        *puVar2 = uVar7 & 0xffff;
        uVar7 = uVar7 + 1;
      }
      uVar4 = uVar4 + 1;
      uVar5 = uVar4 & 0xffff;
    } while (uVar5 < *(uint *)((int)this + 0x30));
  }
  return 0;
}

